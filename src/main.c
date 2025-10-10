//------------------------------------------------------------------------------------
//-- Author: Sylvain Fortin
//-- Project: Halloweenizer
//-- Documentation:
//-- This Raspberry Pi Pico is used to control a MIDI plus a DMX512 interface.
//-- The MIDI output is send to a Roland S-330 digital sampler amplified with speakers.
//-- The DMX512 output is connected to two RGB projectors.
//-- Generated spooky sounds and light effects are intended to amuze the children 
//-- and also the programmer :-)
//------------------------------------------------------------------------------------
#include <stdint.h>
#include "pico/stdlib.h"
#include <string.h> // for strcpy
#include <stdio.h>
#include <stdlib.h> // for atoi
#include <time.h>  // for time in random number generator
#include <math.h>

#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/structs/iobank0.h"

#include "pins.h"
#include "led.h"
#include "uartbuf.h"  // my uart buffer
#include "mystring.h" // my string manipulations
#include "serialprocess.h"
#include "dmx.h"

#define Str_Version "October 10, 2025 11H48"

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

uint32_t MainLoopCount = 0;

//io_irq_ctrl_hw_t *irq_ctrl_base = &iobank0_hw->proc0_irq_ctrl;


typedef struct 
{
   uint8_t intensity;
   uint8_t red;
   uint8_t green;
   uint8_t blue;
} color_t;

// Declare prototype for function
void InterpretCmdString(char *);
void StoreInterpByte(uint8_t);
void send_one_frame(uint8_t Data);

void strobe(uint8_t number, uint16_t period_ms);
void send_color(color_t color, uint16_t time_ms);
void ramp_color(color_t colorA, color_t colorB, uint16_t time_ms);
int GenerateRandomInt (int MaxValue);
void Black(uint16_t time_ms);
void autostrobe(color_t color, uint8_t speed, uint16_t time_ms);

#define PacketSize 20
uint8_t Packet[PacketSize];
void send_packet(uint8_t*);

// Midi routines
void TxMidiByte(uint8_t Data);
void MidiReset(void);
void SoundStart(uint8_t Note, uint8_t Velocity);
void SoundStop(uint8_t Note);
void Tremolo(uint8_t Value);
void PitchBend(int8_t Value);

inline void InlTpDelay()
{
   __asm__ __volatile__(
       "nop\n\t" // 16 ns
       "nop\n\t" // 24 ns
       "nop\n\t" // 31 ns
       "nop\n\t" // 39 ns    For 32 MS/s LA sampling rate
       "nop\n\t" // 45 ns     "  25 MS/s  "    "      "
   );
}


void __not_in_flash_func(MyDelay_ns)(uint16_t Delay_ns)
{
   // uint16_t NbIteration = (uint16_t)Delay_ns / (8 * 4);    // 8 ns per instruction,   4 instructions per loop
   // while(NbIteration-- > 0)
   //{
   //     __asm__ __volatile__ ("nop\n\t");
   //    //NbIteration = NbIteration - 1;
   // }

   __asm__ __volatile__(
       "lsr r0, #5"
       "\n\t" // 8 ns per instruction, 4 instructions per loop => divide by 32
       "loopDelay_ns: sub r0, #1"
       "\n\t" // 1 cycle
       "nop             "
       "\n\t" // 1 cycle
       "bne loopDelay_ns"
       "\n\t" // 2 cycles
   );
}

// RX interrupt handler
void __not_in_flash_func(on_uart_rx)()
{
   while (uart_is_readable(UART_ID))
   {
      char recd = uart_getc(UART_ID);
      // UartBufWr(recd);
      // Cin = UartBufRd();
      ProcessSerialChar(recd);
      if (CmdStringFlag == true)
      {
         CmdStringFlag = false;
         InterpretCmdString(CmdString);
      }
   }
}

void ClearScreen()
{
   printf("\x1B[2J"); // clear screen
   printf("\x1B[H");  // cursor to home
}

// int __not_in_flash_func(main)()
void main() // This enable breakpoint before main loop
{
   stdio_init_all();

   // Set up our UART with a basic baud rate.
   uart_init(UART_ID, 2400);

   // printf("DdcSinkPi Start!\r\n");

   // Set the TX and RX pins by using the function select on the GPIO
   // Set datasheet for more information on function select
   gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
   gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

   // Actually, we want a different speed
   // The call will return the actual baud rate selected, which will be as close as
   // possible to that requested
   int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

   // Set UART flow control CTS/RTS, we don't want these, so turn them off
   uart_set_hw_flow(UART_ID, false, false);

   // Set our data format
   uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

   // Turn off FIFO's - we want to do this character by character
   uart_set_fifo_enabled(UART_ID, false);

   // Set up a RX interrupt
   // We need to set up the handler first
   // Select correct interrupt for the UART we are using
   int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

   // And set up and enable the interrupt handlers
   irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
   irq_set_enabled(UART_IRQ, true);

   // Now enable the UART to send interrupts - RX only
   uart_set_irq_enables(UART_ID, true, false);

   // IO Init
   // Initialize LED GPIO
   gpio_init(LED_GPIO);
   gpio_set_dir(LED_GPIO, GPIO_OUT);

   // Test point (TP)
   gpio_init(TP0_GPIO);
   gpio_set_dir(TP0_GPIO, GPIO_OUT);
   gpio_init(TP1_GPIO);
   gpio_set_dir(TP1_GPIO, GPIO_OUT);
   gpio_init(TP2_GPIO);
   gpio_set_dir(TP2_GPIO, GPIO_OUT);
   gpio_init(TP3_GPIO);
   gpio_set_dir(TP3_GPIO, GPIO_OUT);
   gpio_init(TP4_GPIO);
   gpio_set_dir(TP4_GPIO, GPIO_OUT);
   gpio_init(TP5_GPIO);
   gpio_set_dir(TP5_GPIO, GPIO_OUT);
   gpio_init(TP6_GPIO);
   gpio_set_dir(TP6_GPIO, GPIO_OUT);
   gpio_init(TP7_GPIO);
   gpio_set_dir(TP7_GPIO, GPIO_OUT);
   gpio_init(MidiOut_GPIO);
   gpio_set_dir(MidiOut_GPIO, GPIO_OUT);
   gpio_put(MidiOut_GPIO, 1);   // Force Midi data line to High  (1 Led OFF mean High) (0 Led ON mean Low )
   busy_wait_ms(100);  // Let high level stabilise on midi out

   gpio_init(Data_GPIO);
   gpio_set_dir(Data_GPIO, GPIO_OUT);

   gpio_init(DMX_LED);
   gpio_set_dir(DMX_LED, GPIO_IN);
   gpio_put(DMX_LED, 1);   // DMX LED OFF

   gpio_init(MIDI_LED);
   gpio_set_dir(MIDI_LED, GPIO_IN);
   gpio_put(MIDI_LED, 1);   // MIDI LED OFF

   /*
   // Do a couple of blinking on DMX and MIDI LED
   for(uint8_t i = 0; i < 8; i++)
   {
      SetDmxLed(true);   // DMX LED ON
      busy_wait_ms(100);
      SetDmxLed(false);   // DMX LED OFF
      busy_wait_ms(100);
   }

   for(uint8_t i = 0; i < 8; i++)
   {
      SetMidiLed(true);   // MIDI LED ON
      busy_wait_ms(100);
      SetMidiLed(false);   // MIDI LED OFF
      busy_wait_ms(100);
   }
   */

   ClearScreen();
   printf("HALLOWEENIZER\r\n");
   printf("DMX and MIDI controller for Raspberry Pi Pico\r\n");
   printf("Firmware Version : %s\r\n", Str_Version);

   CmdStringFlag = false;
   AnsiEscapeState = 0;
   CmdHistSel = 0;
   InitHistory();

   CmdCharCnt = 0;
   //printf(">"); // prompt

   uint8_t TestCount = 0;

   color_t colorA, colorB, colorC;

   bool Timereach = false;
   uint64_t TimeStartUs;
   uint64_t delta_t_us;
   uint16_t time_ms;

   // Loop forever
   while (true)
   {
      printf("----------\r\n");
      printf("Loop BEGIN\r\n");
      printf("----------\r\n");

      // Midi test
      MidiReset();
      uint8_t Note;
      //while(1)
      //{
         //Note = 36;   // C2 Organ   
         //Note = 37;   // C#2 Organ   
         //Note = 38;   // D2 GLutural Water sink lower frequency pitch
         //Note = 39;   // D#2 Organ   
         //Note = 40;   // E2 Organ   
         //Note = 41;   // F2 Organ   
         //Note = 42;   // F#2 GLutural Water sink frequency pitch
         //Note = 43;   // G2 Laph Very lower frequency pitch
         //Note = 44;   // G#2 Organ
         //Note = 45;   // A2 Beast Glutural Lower Pitch
         //Note = 46;   // A#2 Organ
         //Note = 47;   // B2 Hurlement type syrene lower pitch (one shot)
         //Note = 48;   // C3 Hurlement type syrene lower pitch (one shot)
         //Note = 49;   // C#3 Organ
         //Note = 50;   // D3 Laph Very low frequency pitch
         //Note = 51;   // D#3 Beast Lower
         //Note = 52;   // E3 Hurlement type syrene lower pitch (one shot)
         //Note = 53;   // F3 Organ
         //Note = 54;   // F#3 Hurlement type syrene (one shot)
         //Note = 55;   // G3 Beast Glutural Low
         //Note = 56;   // G#3 Organ
         //Note = 57;   // A3 Thunderstorm (one shot)
         //Note = 58;   // A#3 Organ
         //Note = 59;   // B3 Organ
         //Note = 60;   // C4 Rire lent (joue pendant l'appuie)
         //Note = 61;   // C#4 Organ
         //Note = 62;   // D4 Organ
         //Note = 63;   // D#4 Monster Whaaa
         //Note = 64;   // E4 Hurlement type syrene higher pitch (one shot)
         //Note = 65;   // F4 Monster Whaaa higher pitch
         //Note = 66;   // F#4 Organ
         //Note = 67;   // G4 Organ
         //Note = 68;   // G#4 Organ
         //Note = 69;   // A Monster Classic
         //Note = 70;   // A#4 Organ
         //Note = 71;   // B4 Organ
         //Note = 72;   // C5 Beast Glutural High
         //Note = 73;   // C#5 Monster Whaaa higher pitch
         //Note = 74;   // D5 Wild animal
         //Note = 75;   // D#5 Gun shot (one shot)
         //Note = 76;   // E5 Wild animal higher pitch
         //Note = 77;   // F5 Crying body (one shot)
         //Note = 78;   // F#5 Wild animal higher pitch
         //Note = 79;   // G5 Crying body (one shot)
         //Note = 80;   // G#5 Organ
         //Note = 81;   // A5 Organ
         //Note = 82;   // A#5 Laught High pitch
         //Note = 83;   // B5 Wild animal higher pitch
         //Note = 84;   // C6 Wild animal higher pitch
         //Note = 85;   // C#6 -
         //Note = 86;   // D6 Wild animal higher pitch
         //Note = 87;   // D#6 Crying female (one shot)
         //Note = 88;   // E6 Croincement court
         //Note = 89;   // F6 Laught child
         //Note = 90;   // F#6 Wild animal higher pitch
         //Note = 91;   // G6 Croincement court 2
         //Note = 92;   // G#6 Blody bird
         //Note = 93;   // A6 Monster children Whaaa higher pitch
         //Note = 94;   // A#6 Croincement court 3
         //Note = 95;   // B6 Animal crying Highly

         //SoundStart(Note,82);
         //busy_wait_ms(1000);
         //SoundStop(Note);
         //busy_wait_ms(5000);
      //}

      // Halloween orange
      printf("Halloween orange\r\n");
      Note = 47;   // B2 Hurlement type syrene lower pitch (one shot)
      SoundStart(Note,127);
      colorA.intensity = 255;
      colorA.red = 0;
      colorA.green = 0;
      colorA.blue = 0;
      colorB.intensity = 255;
      colorB.red = 255;
      colorB.green = 23;
      colorB.blue = 2;
      ramp_color(colorA, colorB, 4000);
      //send_color(colorB, 13000);
      // play organ for a while
      uint8_t my_note_array[] = {36, 37, 39, 40, 41, 44, 46, 49, 53, 56, 58, 59, 61, 62, 66, 67, 68, 70, 71, 80, 81};
      Timereach = false;
      TimeStartUs = time_us_64(); // Time snapshot
      time_ms = 20000;
      Tremolo(0x7F);  // Max
      while (Timereach == false) // keep sending packets as long as not reached time period?
      {
         delta_t_us = time_us_64() - TimeStartUs;
         uint8_t OrganRandomNumber = GenerateRandomInt(20);
         uint8_t OrganRandomNote = my_note_array[OrganRandomNumber];
         uint16_t OrganRandomDuration = GenerateRandomInt(1500);
         uint8_t OrganRandomVelocity = GenerateRandomInt(127);
         SoundStart(OrganRandomNote,OrganRandomVelocity);
         //busy_wait_ms(300);
         send_color(colorB, OrganRandomDuration);
         SoundStop(OrganRandomNote);
         if( delta_t_us > (time_ms * 1000))
         {
            Timereach = true;
         }
      }
      Tremolo(0);  // OFF
      SoundStop(Note);
      SoundStart(Note,127);
      ramp_color(colorB, colorA, 4000);
      SoundStop(Note);
      Black(2000); 

      // Automatic color sudent change
      printf("Automatic color change\r\n");
      SoundStart(43,127);  // G2 Laph Very lower frequency pitch
      SoundStart(77,127);  // F5 Crying body (one shot)
      Timereach = false;
      TimeStartUs = time_us_64(); // Time snapshot
      time_ms = 20000;
      Packet[0] = 0;
      Packet[1] = 0;    // intensity
      Packet[2] = 0;    // red
      Packet[3] = 0;    // green
      Packet[4] = 0;    // blue
      Packet[5] = 0;
      Packet[6] = 200;    // Function selection
                           // 254, 255 sound control
                           // 200 Color change
                           // 100, 151 Color change gradually
                           // 20, 39, 51 Turn on
                           // 0, 1, 10 Strobe
      while (Timereach == false) // keep sending packets as long as not reached time period?
      {
         delta_t_us = time_us_64() - TimeStartUs;
         send_packet(Packet);
         if( delta_t_us > (time_ms * 1000))
         {
            Timereach = true;
         }
      }
      SoundStop(43);
      SoundStop(77);
      Black(2000);

      // white ramp up down
      printf("White ramp up down\r\n");
      colorA.intensity = 255;
      colorA.red = 0;
      colorA.green = 0;
      colorA.blue = 0;
      colorB.intensity = 255;
      colorB.red = 255;
      colorB.green = 255;
      colorB.blue = 255;
      ramp_color(colorA, colorB, 1000);   // ramp up 
      ramp_color(colorB, colorA, 2000);   // ramp down
      busy_wait_ms(2000);
      ramp_color(colorA, colorB, 800);   // ramp up 
      ramp_color(colorB, colorA, 3000);   // ramp down
      busy_wait_ms(2000);
      ramp_color(colorA, colorB, 700);   // ramp up 
      ramp_color(colorB, colorA, 4000);   // ramp down
      busy_wait_ms(2000);

      // yello candy treets
      printf("Yellow candy treets\r\n");
      SoundStart(79,127);  // G5 Crying body (one shot)
      busy_wait_ms(1000);
      SoundStop(79);
      colorA.intensity = 255;    // Yellow
      colorA.red = 255;
      colorA.green = 99;
      colorA.blue = 0;
      colorB.intensity = 255;    // Black
      colorB.red = 0;
      colorB.green = 0;
      colorB.blue = 0;
      send_color(colorB, 1);
      ramp_color(colorB, colorA, 4000);
      SoundStart(77,127);  // F5 Crying body (one shot)
      send_color(colorA, 13000);
      SoundStop(77);
      ramp_color(colorA, colorB, 4000);
      SoundStart(79,127);  // G5 Crying body (one shot)
      busy_wait_ms(1000);
      SoundStart(79,127);  // G5 Crying body (one shot)
      Black(2000);
      SoundStop(79);
      SoundStop(77);

      // Normal light
      printf("Normal light\r\n");
      SoundStart(48,70);   // C3 Hurlement type syrene lower pitch (one shot)
      busy_wait_ms(1000);
      SoundStart(36,100);  // C2 Organ
      Tremolo(0x7F); // Max tremolo
      colorA.intensity = 255;
      colorA.red = 255;
      colorA.green = 255;
      colorA.blue = 255;
      send_color(colorA, 10000);
      SoundStop(36);
      SoundStop(48);
      Tremolo(0); // tremolo stop

      // Defective light (black glitch)
      printf("Defective light (black glitch)\r\n");
      SoundStart(50,70);   // D3 Laph Very low frequency pitch
      busy_wait_ms(1000);
      SoundStart(36,100);  // C2 Organ
      Tremolo(0x7F); // Max tremolo
      colorA.intensity = 255;
      colorA.red = 255;
      colorA.green = 255;
      colorA.blue = 255;
      colorB.intensity = 0;
      colorB.red = 0;
      colorB.green = 0;
      colorB.blue = 0;
      for (int iter = 0; iter < 15; iter++)
      {
         int RandomPitchBend = (GenerateRandomInt(16)*4-1)-64;
         PitchBend(RandomPitchBend);
         send_color(colorA, GenerateRandomInt(3000)); // random ON time
         for (int n = 0; n < GenerateRandomInt(7); n++)  // random number of glitch
         {
            send_color(colorB, GenerateRandomInt(100)); // random OFF time
            send_color(colorA, GenerateRandomInt(150)); // random ON time
         }
      }
      SoundStop(50);
      SoundStop(36);
      PitchBend(0);
      Tremolo(0); // tremolo stop

      // Cold purple black light
      printf("Cold purple black light\r\n");
      SoundStart(63,100);  //D#4 Monster Whaaa
      busy_wait_ms(2000);
      SoundStop(63);
      SoundStart(64,100); // E4 Hurlement type syrene higher pitch (one shot)
      colorA.intensity = 255;
      colorA.red = 0;
      colorA.green = 0;
      colorA.blue = 0;
      colorB.intensity = 255;
      colorB.red = 255;
      colorB.green = 0;
      colorB.blue = 255;
      ramp_color(colorA, colorB, 4000);
      send_color(colorB, 8000);
      ramp_color(colorB, colorA, 4000);
      SoundStop(64);

      // Red hearth beat
      printf("Red hearth beat\r\n");
      SoundStart(60,127); // C4 Rire lent (joue pendant l'appuie)
      busy_wait_ms(1000);
      SoundStop(60);
      SoundStart(43,127); // G2 Laph Very lower frequency pitch
      colorA.intensity = 255;
      colorA.red = 0;
      colorA.green = 0;
      colorA.blue = 0;
      colorB.intensity = 255;
      colorB.red = 255;
      colorB.green = 0;
      colorB.blue = 0;
      for (int n = 0; n<10; n++)
      {
         ramp_color(colorA, colorB, 100);   // ramp up
         ramp_color(colorB, colorA, 90);   // ramp down
         busy_wait_ms(40);
         ramp_color(colorA, colorB, 130);   // ramp up
         ramp_color(colorB, colorA, 140);   // ramp down
         busy_wait_ms(800);
      }
      SoundStop(43);

      // Wavy color green
      printf("Wavy color green\r\n");
      Note = 60;   // C4 Rire lent (joue pendant l'appuie)
      SoundStart(Note,90);
      colorC.intensity = 255;
      colorC.red = 0;
      colorC.green = 0;
      colorC.blue = 0;
      colorA.intensity = 255;
      colorA.red = 0;
      colorA.green = 128;
      colorA.blue = 0;
      ramp_color(colorC, colorA, 3000);
      SoundStop(Note);
      color_t colorX;
      delta_t_us;
      Timereach = false;
      TimeStartUs = time_us_64(); // Time snapshot
      time_ms = 10000;
      float FrequencyIni = 0.75;    // make the frequency change uop and down as time goes
      float Frequency;
      uint16_t Amplitude = 127; 
      Note = 82;   // A#5 Laught High pitch
      SoundStart(Note,90);
      while (Timereach == false) // keep sending packets as long as not reached time period?
      {
         delta_t_us = time_us_64() - TimeStartUs;
         Frequency = FrequencyIni + FrequencyIni * sin((2*3.1416*0.05*(float)delta_t_us)/1000000);
         colorX.intensity = colorA.intensity;
         colorX.red = colorA.red;
         colorX.green = (uint8_t)(colorA.green + Amplitude * sin((2*3.1416*Frequency*(float)delta_t_us)/1000000));
         colorX.blue = colorA.blue;
         Packet[0] = 0;
         Packet[1] = colorX.intensity; 
         Packet[2] = colorX.red;       
         Packet[3] = colorX.green;     
         Packet[4] = colorX.blue;      
         Packet[5] = 0;
         Packet[6] = 0;    // Function selection
                           // 254, 255 sound control
                           // 200 Color change
                           // 100, 151 Color change gradually
                           // 20, 39, 51 Turn on
                           // 0, 1, 10 Strobe
         send_packet(Packet);
         if( delta_t_us > (time_ms * 1000))
         {
            Timereach = true;
         }
      }
      SoundStop(Note);
      Note = 60;   // C4 Rire lent (joue pendant l'appuie)
      SoundStart(Note,90);
      Black(2000);
      SoundStop(Note);

      // Defective light (random color glitch)
      printf("Defective light (random color glitch)\r\n");
      Note = 54;   // F#3 Hurlement type syrene (one shot)
      SoundStart(Note,127);
      busy_wait_ms(50);
      SoundStop(Note);
      colorA.intensity = 255;
      colorA.red = 255;    // normal white light color
      colorA.green = 255;
      colorA.blue = 255;
      for (int iter = 0; iter < 15; iter++)
      {
         send_color(colorA, GenerateRandomInt(3000)); // random ON time
         Note = 55;   // G3 Beast Glutural Low
         SoundStart(Note,127);
         for (int n = 0; n < GenerateRandomInt(7); n++)  // random number of glitch
         {
            colorB.red = GenerateRandomInt(255);   // paranormal random color
            colorB.green = GenerateRandomInt(255);
            colorB.blue = GenerateRandomInt(255);
            colorB.intensity = GenerateRandomInt(255);
            send_color(colorB, GenerateRandomInt(150)); // random paranormal color time
            send_color(colorA, GenerateRandomInt(100)); // random ON time
         }
         SoundStop(Note);
      }
      colorB.intensity = 0;
      colorB.red = 0;
      colorB.green = 0;
      colorB.blue = 0;
      send_color(colorB, 5000); // Light goes abruptly dead  

      // Random color quick change
      printf("Random color quick change\r\n");
      uint8_t my_randomColorSound_array[] = {90, 91, 93, 95};
      for (int iter = 0; iter < 8; iter++)
      {
         uint8_t Number_Of_Color_Flash = GenerateRandomInt(25);
         uint8_t randomColorSound = my_randomColorSound_array[GenerateRandomInt(3)];
         SoundStart(randomColorSound,60);
         for (int n = 0; n < Number_Of_Color_Flash; n++)
         {
            colorA.intensity = GenerateRandomInt(255);
            colorA.red = GenerateRandomInt(255);
            colorA.green = GenerateRandomInt(255);
            colorA.blue = GenerateRandomInt(255);
            send_color(colorA, GenerateRandomInt(200));   // Red 
         }  
         SoundStop(randomColorSound);
         colorA.intensity = 255;
         colorA.red = 0;
         colorA.green = 0;
         colorA.blue = 0;
         send_color(colorA, GenerateRandomInt(4000));  // Black for random duration
      }

      // Pale skin
      printf("Pale skin\r\n");
      Note = 38;   // D2 GLutural Water sink lower frequency pitch
      SoundStart(Note,60);
      colorA.intensity = 255;
      colorA.red = 0;
      colorA.green = 0;
      colorA.blue = 0;
      colorB.intensity = 255;
      colorB.red = 218;
      colorB.green = 42;
      colorB.blue = 14;
      colorC.intensity = 255;
      colorC.red = 255;
      colorC.green = 12;
      colorC.blue = 0;
      ramp_color(colorA, colorB, 4000);
      SoundStop(Note);
      Note = 42;   // F#2 GLutural Water sink frequency pitch
      SoundStart(Note,100);
      ramp_color(colorB, colorC, 4000);
      //send_color(colorB, 4000);
      SoundStop(Note);
      Note = 38;   // D2 GLutural Water sink lower frequency pitch
      SoundStart(Note,90);
      ramp_color(colorC, colorB, 4000);
      SoundStop(Note);
      Note = 42;   // F#2 GLutural Water sink frequency pitch
      SoundStart(Note,127);
      ramp_color(colorB, colorC, 4000);
      SoundStop(Note);
      Note = 38;   // D2 GLutural Water sink lower frequency pitch
      SoundStart(Note,90);
      ramp_color(colorC, colorB, 4000);
      SoundStop(Note);
      Note = 42;   // F#2 GLutural Water sink frequency pitch
      SoundStart(Note,50);
      ramp_color(colorB, colorA, 4000);
      SoundStop(Note);
      Black(2000);
      
      // Cold blue
      printf("Cold blue\r\n");
      Note = 43;   // G2 Laph Very lower frequency pitch
      SoundStart(Note,127);
      colorA.intensity = 255;
      colorA.red = 0;
      colorA.green = 0;
      colorA.blue = 0;
      colorB.intensity = 255;
      colorB.red = 0;
      colorB.green = 247;
      colorB.blue = 255;
      ramp_color(colorA, colorB, 4000);
      send_color(colorB, 13000);
      ramp_color(colorB, colorA, 4000);
      Black(2000);
      SoundStop(Note);

      // Thunderstorm lightning effect using white flash strobes
      printf("Thunderstorm lightning effect\r\n");
      colorA.intensity = 255; // Black
      colorA.red = 0;
      colorA.green = 0;
      colorA.blue = 0;
      colorB.intensity = 255; // Gray day light
      colorB.red = 20;
      colorB.green = 23;
      colorB.blue = 27;
      ramp_color(colorA, colorB, 5000);
      uint8_t ThunderMinNumberFlash = 1;
      uint8_t ThunderMaxNumberFlash = 5;
      uint16_t ThunderMaxPeriod_ms = 200;
      uint16_t ThunderMinPeriod_ms = 50;
      Note = 57;   // A3 Thunderstorm (one shot)
      for (int iter = 0; iter < 12; iter++)
      {
         uint8_t Number_Of_Flash = (uint8_t)GenerateRandomInt(ThunderMaxNumberFlash-ThunderMinNumberFlash)+ThunderMinNumberFlash;
         uint16_t ThunderFlashPeriod = GenerateRandomInt(ThunderMaxPeriod_ms-ThunderMinPeriod_ms)+ThunderMinPeriod_ms;
         uint8_t ThunderSoundIntensity = (uint8_t)(127*Number_Of_Flash/ThunderMaxNumberFlash);
         SoundStart(Note,ThunderSoundIntensity);
         SoundStop(Note);
         strobe(Number_Of_Flash, ThunderFlashPeriod);
         //busy_wait_ms(GenerateRandomInt(4000));
         send_color(colorB, 2000+GenerateRandomInt(4000));
      }
      ramp_color(colorB, colorA, 5000);

      // Blue dark modulation
      printf("Blue dark modulation\r\n");
      colorA.intensity = 255;
      colorA.red = 0;
      colorA.green = 0;
      colorA.blue = 0;
      colorB.intensity = 255;
      colorB.red = 0;
      colorB.green = 0;
      colorB.blue = 255;
      ramp_color(colorA, colorB, 5000);   // slow ramp up 
      colorA.blue = 40;
      for (int n = 0; n<7; n++)
      {
         uint8_t BlueSoundIntensity = (uint8_t)120*(n+1)/8;
         SoundStart(74,BlueSoundIntensity);  // D5 Wild animal
         ramp_color(colorB, colorA, 2000);   // half fade
         SoundStart(76,BlueSoundIntensity+10);  // E5 Wild animal higher pitch
         ramp_color(colorA, colorB, 2000);   // back to blue
         SoundStop(74);
         SoundStop(76);
      }
      colorA.blue = 0;
      ramp_color(colorB, colorA, 5000);   //fade out

      // White flash strobes on red background
      printf("White flash strobes on red background\r\n");
      Note = 43;   // G2 Laph Very lower frequency pitch
      SoundStart(Note,127);
      colorA.intensity = 255;    // Red background
      colorA.red = 255;
      colorA.green = 0;
      colorA.blue = 0;
      send_color(colorA, 1000);  // White
      colorB.intensity = 255;
      colorB.red = 255;
      colorB.green = 255;
      colorB.blue = 255;
      colorC.intensity = 255;    // Black
      colorC.red = 0;
      colorC.green = 0;
      colorC.blue = 0;
      for (int iter = 0; iter < 8; iter++)
      {
         SoundStart(77,40);  // F5 Crying body (one shot)
         send_color(colorB, 60);    // White strobe
         //send_color(colorC, 100);   // Black
         //send_color(colorA, 700);  // Red
         ramp_color(colorC, colorA, 200);
         //strobe(3, 150);
         send_color(colorA, 1000);   // Red
      }
      SoundStop(77);
      SoundStop(Note);
      Black(2000);

      // burning
      printf("Burning\r\n");
      colorA.red = 0;
      colorA.green = 0;
      colorA.blue = 0;
      colorB.intensity = 255; // Halloween orange
      colorB.red = 255;
      colorB.green = 23;
      colorB.blue = 2;
      colorC.intensity = 255; // Full Red
      colorC.red = 255;
      colorC.green = 0;
      colorC.blue = 0;
      ramp_color(colorA, colorB, 3000);   // Initial ramp up
      Note = 77;   // F5 Crying body (one shot)
      SoundStart(Note,127);
      SoundStop(Note);
      Note = 38;   // D2 GLutural Water sink lower frequency pitch
      SoundStart(Note,127);
      delta_t_us;
      Timereach = false;
      TimeStartUs = time_us_64(); // Time snapshot
      time_ms = 20000;
      while (Timereach == false) // keep sending packets as long as not reached time period?
      {
         delta_t_us = time_us_64() - TimeStartUs;

         ramp_color(colorB, colorC, GenerateRandomInt(400));
         ramp_color(colorC, colorB, GenerateRandomInt(400));

         if( delta_t_us > (time_ms * 1000))
         {
            Timereach = true;
         }
      }
      Note = 75;   // D#5 Gun shot (one shot)
      SoundStart(Note,100);
      SoundStop(Note);
      ramp_color(colorB, colorA, 3000);   // Ramp down
      Black(3000);

      // Automatic strobe
      printf("Automatic strobe\r\n");
      //void autostrobe(color_t color, uint8_t speed, uint16_t time_ms);  
      colorA.intensity = 255; // Blue
      colorA.red = 0;
      colorA.green = 0;
      colorA.blue = 255;
      Note = 36; // C2 Organ
      SoundStart(Note,80);
      autostrobe(colorA, 100, 3000);
      SoundStop(Note);
      Black(2000);
      colorA.intensity = 255; // Red
      colorA.red = 255;
      colorA.green = 0;
      colorA.blue = 0;
      Note = 37; // C#2 Organ 
      SoundStart(Note,80);
      autostrobe(colorA, 255, 1000);
      SoundStop(Note);
      Black(2000);
      colorA.intensity = 255; // Green
      colorA.red = 0;
      colorA.green = 255;
      colorA.blue = 0;
      Note = 40; // E2 Organ  
      SoundStart(Note,80);
      autostrobe(colorA, 140, 2000);
      SoundStop(Note);
      Black(1500);
      colorA.intensity = 255; // Red + Blue
      colorA.red = 255;
      colorA.green = 0;
      colorA.blue = 255;
      Note = 41;   // F2 Organ 
      SoundStart(Note,80);
      autostrobe(colorA, 170, 1000);
      SoundStop(Note);
      Black(2200);
      colorA.intensity = 255; // Red + Green
      colorA.red = 255;
      colorA.green = 255;
      colorA.blue = 0;
      Note = 44;   // G#2 Organ
      SoundStart(Note,80);
      autostrobe(colorA, 190, 1000);
      SoundStop(Note);
      Black(1500);
      colorA.intensity = 255; // Green + Blue
      colorA.red = 0;
      colorA.green = 255;
      colorA.blue = 255;
      Note = 46;   // A#2 Organ
      SoundStart(Note,80);
      autostrobe(colorA, 210, 1000);
      SoundStop(Note);
      Black(2500);
      colorA.intensity = 255; // White
      colorA.red = 255;
      colorA.green = 255;
      colorA.blue = 255;
      Note = 49;   // C#3 Organ
      SoundStart(41,80);   // F2 Organ  
      SoundStart(59,80);   // B3 Organ  
      SoundStart(62,80);   // D4 Organ  
      SoundStart(70,80);   // A#4 Organ
      autostrobe(colorA, 250, 2500);
      SoundStop(41);
      SoundStop(59);
      SoundStop(62);
      SoundStop(70);
      Black(3000);

      // Automatic color gradual change
      printf("Automatic color gradual change\r\n");
      Timereach = false;
      TimeStartUs = time_us_64(); // Time snapshot
      time_ms = 20000;
      Packet[0] = 0;
      Packet[1] = 0;    // intensity
      Packet[2] = 0;    // red
      Packet[3] = 0;    // green
      Packet[4] = 0;    // blue
      Packet[5] = 0;
      Packet[6] = 100;    // Function selection
                           // 254, 255 sound control
                           // 200 Color change
                           // 100, 151 Color change gradually
                           // 20, 39, 51 Turn on
                           // 0, 1, 10 Strobe
      while (Timereach == false) // keep sending packets as long as not reached time period?
      {
         delta_t_us = time_us_64() - TimeStartUs;
         send_packet(Packet);
         if( delta_t_us > (time_ms * 1000))
         {
            Timereach = true;
         }
      }
      Black(3000);

      // Led toggle.  Take 440ns to process!
      // if ((time_us_64() - LedToggleTimeStartUs) > (LedToggleTime_ms * 1000)) // Reached led toggle time period?
      //{
      //    gpio_xor_mask(1 << LED_GPIO);
      //    LedToggleTimeStartUs = time_us_64(); // Time snapshot for led toggle
      //}

      // Led toggle.  Take 160ns to process. Led Blink do not need to be precise
      if (MainLoopCount >= 1 / (4 * 438e-9)) // 1/(4*looptime) for 2 blink per second
      {                                      // Take 160ns to execute
         gpio_xor_mask(1 << LED_GPIO);
         MainLoopCount = 0;
      }
      MainLoopCount++;

      //busy_wait_us(20000);
      busy_wait_us(1000);

      // Toggle IO to show main loop running
      gpio_xor_mask(1 << TP7_GPIO);
   }
}


void InterpretCmdString(char *CmdStr)
{
   char OneChar;
   if (CmpStrEqu(CmdStr, "cls") == true)
   {
      ClearScreen();
   }
   else if (CmpStrEqu(CmdStr, "history") == true)
   {
      for (unsigned char HistCnt = CMD_HIST_SIZE; HistCnt > 0; HistCnt--)
      {
         printf("\n\r");
         printf("%-4d", CMD_HIST_SIZE - HistCnt + 1);
         // printf("  ");
         printf("%s", CmdHistory[HistCnt - 1]);
      }
      printf("\n\r");
   }
   else if (CmpStrEqu(CmdStr, "!") == true)
   {
      printf("\n\r");
      char *Pos = strchr(CmdStr, '!'); // position of ! character
      Pos++;
      u_int8_t value = atoi(Pos);
      if ((value >= 1) & (value <= CMD_HIST_SIZE))
      {
         CpHistToCmd(CMD_HIST_SIZE - value); // just copy to cmd without pushing into history stack
         InterpretCmdString(CmdStr);         // recursive call to process the recalled command
      }
      else
      {
         printf("Error : Value outside history range\n\r");
      }
   }
   else if (CmpStrEqu(CmdStr, "reset") == true) // soft reset
   {
      // printf("\033c");
      ClearScreen();
   }
   else if (CmpStrEqu(CmdStr, "ver") == true)
   {
      printf("\n\rFirmware Version : %s\n\r", Str_Version);
   }
   else if ((CmpStrEqu(CmdStr, "help") == true) | (CmpStrEqu(CmdStr, "?") == true))
   {
      printf("\n\rHALLOWEENIZER help menu\r\n");
      printf("stop            Stop the sequence\r\n");
      printf("start           Start the sequence\r\n");
      printf("reset           Software reset\r\n");
      printf("cls             Clear screen\r\n");
      printf("history         History of commands\r\n");
      printf("!n              Refer to command line n\r\n");
      printf("reset           Software reset\r\n");
      printf("ver             Display firmware version\r\n");
      printf("help | ?        Help for commands\r\n");
   }
   else
   {
      if (CmdStr[0]) // Some character present ?
      {
         printf("\n\rCommand '%s' not found.\n\r", CmdStr);
      }
      else // nothing was typed
      {
         printf("\n\r"); // next line
      }
   }
   printf(">"); // Prompt
   CmdCharCnt = 0;
   ClearCmd();
}

void strobe(uint8_t number, uint16_t period_ms)
{
      uint8_t n = number;
      for(int n = 0; n < number; n++)
      {
         Packet[0] = 0;
         Packet[1] = 255; // intensity
         Packet[2] = 255; // Red
         Packet[3] = 255; // Green
         Packet[4] = 255; // Blue
         Packet[5] = 0;
         Packet[6] = 0;    // Function selection
                           // 254, 255 sound control
                           // 200 Color change
                           // 100, 151 Color change gradually
                           // 20, 39, 51 Turn on
                           // 0, 1, 10 Strobe
         send_packet(Packet);
         busy_wait_ms(16);

         Packet[1] = 0; // intensity
         Packet[2] = 0; // Red
         Packet[3] = 0; // Green
         Packet[4] = 0; // Blue

         send_packet(Packet);

         busy_wait_ms(period_ms);
      }
}

void send_color(color_t color, uint16_t time_ms)
{
   uint64_t TimeStartUs = time_us_64(); // Time snapshot

   while ((time_us_64() - TimeStartUs) < (uint64_t)(time_ms * 1000)) // keep sending packets as long as not reached time period?
   {
      Packet[0] = 0;
      Packet[1] = color.intensity;  // intensity
      Packet[2] = color.red;     // Red
      Packet[3] = color.green;   // Green
      Packet[4] = color.blue;    // Blue
      Packet[5] = 0;
      Packet[6] = 0;    // Function selection
                        // 254, 255 sound control
                        // 200 Color change
                        // 100, 151 Color change gradually
                        // 20, 39, 51 Turn on
                        // 0, 1, 10 Strobe
      send_packet(Packet);
   }
}

void ramp_color(color_t colorA, color_t colorB, uint16_t time_ms)
{
   // delta Y
   float delta_intensity = (float)colorB.intensity - (float)colorA.intensity;
   float delta_red = (float)colorB.red - (float)colorA.red;
   float delta_green = (float)colorB.green - (float)colorA.green;
   float delta_blue = (float)colorB.blue - (float)colorA.blue;
   // delta Y / delta X (slope)
   float m_intensity = (float)delta_intensity/(float)(1000*time_ms);
   float m_red = (float)delta_red/(float)(1000*time_ms);
   float m_green = (float)delta_green/(float)(1000*time_ms);
   float m_blue = (float)delta_blue/(float)(1000*time_ms);
   
   color_t colorX;

   uint64_t delta_t_us;
   bool Timereach = false;
   uint64_t TimeStartUs = time_us_64(); // Time snapshot
   while (Timereach == false) // keep sending packets as long as not reached time period?
   {
      delta_t_us = time_us_64() - TimeStartUs;

      colorX.intensity = (uint8_t)(m_intensity * delta_t_us + (float)colorA.intensity);
      colorX.red = (uint8_t)(m_red * delta_t_us + (float)colorA.red);
      colorX.green = (uint8_t)(m_green * delta_t_us + (float)colorA.green);
      colorX.blue = (uint8_t)(m_blue * delta_t_us + (float)colorA.blue);

      Packet[0] = 0;
      Packet[1] = colorX.intensity; 
      Packet[2] = colorX.red;       
      Packet[3] = colorX.green;     
      Packet[4] = colorX.blue;      
      Packet[5] = 0;
      Packet[6] = 0;    // Function selection
                        // 254, 255 sound control
                        // 200 Color change
                        // 100, 151 Color change gradually
                        // 20, 39, 51 Turn on
                        // 0, 1, 10 Strobe
      send_packet(Packet);
      if( delta_t_us > (time_ms * 1000))
      {
         Timereach = true;
      }
   }
}

//    Random value will be from 0 to #
int GenerateRandomInt (int MaxValue)
{
    unsigned int iseed = (unsigned int)time(NULL);          //Seed srand() using time() otherwise it will start from a default value of 1
    //srand (iseed);
    int random_value = (int)((1.0 + MaxValue) * rand() / ( RAND_MAX + 1.0 ) );      //Scale rand()'s return value against RAND_MAX using doubles instead of a pure modulus to have a more distributed result.
    return(random_value);
}

void Black(uint16_t time_ms)
{
   color_t colorA;
   colorA.intensity = 255;    // Black
   colorA.red = 0;
   colorA.green = 0;
   colorA.blue = 0;
   send_color(colorA, time_ms);
}

// White flash strobe using the automatic strober
// speed: 8 slowest, 255 fastest
void autostrobe(color_t color, uint8_t speed, uint16_t time_ms)
{
   bool Timereach = false;
   uint64_t TimeStartUs = time_us_64(); // Time snapshot
   uint64_t delta_t_us;
   Packet[0] = 0;
   Packet[1] = color.intensity;  // intensity of strobe
   Packet[2] = color.red;     // red strobe component
   Packet[3] = color.green;   // green "        "
   Packet[4] = color.blue;    // blue  "        "
   Packet[5] = speed;         // 0-7: RGBlight control
                              // 8, 9, 10 strobe slow speed
                              // 128: strobe medium speed
                              // 255: strobe fast speed
   Packet[6] = 10;   // Function selection
                     // 254, 255 sound control
                     // 200 Color change
                     // 100, 151 Color change gradually
                     // 20, 39, 51 Turn on
                    // 0, 1, 10 Strobe
   while (Timereach == false) // keep sending packets as long as not reached time period?
   {
      delta_t_us = time_us_64() - TimeStartUs;
      send_packet(Packet);
      if( delta_t_us > (time_ms * 1000))
      {
         Timereach = true;
      }
   }
}


// Transmit one byte to Midi Output
//  ------------------------------------------------------
// | START | D0 | D1 | D2 | D3 | D4 | D5 | D6 | D7 | STOP |
//  ------------------------------------------------------
// START is 0
// STOP is 1
// MidiOut_GPIO : (Led OFF mean High) (Led ON mean Low )
// 31250 baud, 32us bit period
void TxMidiByte(uint8_t Data)
{
   SetMidiLed(true); // Led ON during transmission
   gpio_put(MidiOut_GPIO, 0); // START
   busy_wait_us(32);
   
   (Data & 0x01) ? gpio_put(MidiOut_GPIO, 1) : gpio_put(MidiOut_GPIO, 0);  // D0
   busy_wait_us(32);

   (Data & 0x02) ? gpio_put(MidiOut_GPIO, 1) : gpio_put(MidiOut_GPIO, 0);  // D1
   busy_wait_us(32);

   (Data & 0x04) ? gpio_put(MidiOut_GPIO, 1) : gpio_put(MidiOut_GPIO, 0);  // D2
   busy_wait_us(32);

   (Data & 0x08) ? gpio_put(MidiOut_GPIO, 1) : gpio_put(MidiOut_GPIO, 0);  // D3
   busy_wait_us(32);
   
   (Data & 0x10) ? gpio_put(MidiOut_GPIO, 1) : gpio_put(MidiOut_GPIO, 0);  // D4
   busy_wait_us(32);

   (Data & 0x20) ? gpio_put(MidiOut_GPIO, 1) : gpio_put(MidiOut_GPIO, 0);  // D5
   busy_wait_us(32);

   (Data & 0x40) ? gpio_put(MidiOut_GPIO, 1) : gpio_put(MidiOut_GPIO, 0);  // D6
   busy_wait_us(32);

   (Data & 0x80) ? gpio_put(MidiOut_GPIO, 1) : gpio_put(MidiOut_GPIO, 0);  // D7
   busy_wait_us(32);

   gpio_put(MidiOut_GPIO, 1);
   busy_wait_us(32);        // STOP last one bit

   SetMidiLed(false); // Led ON during transmission
}
void MidiReset(void)
{
   printf("Midi Reset\r\n");
   // To attempt resynchronization
   TxMidiByte(0x80);    // Midi interface init with two note OFF
   TxMidiByte(60);      // 0x3C Middle C
   TxMidiByte(0x64);
   busy_wait_ms(100);
   //
   TxMidiByte(0x80);
   TxMidiByte(60);      // 0x3C  Middle C
   TxMidiByte(0x64);
   busy_wait_ms(100);
   //
   //TxMidiByte(0xFF);    // Midi reset this approach does not work i dont even see the trace on the Roland S330
   //
   // Alternative: General MIDI Reset (SysEx) This approach is not working on the Roland S330
   //TxMidiByte(0xF0);    // F0: Start of SysEx
   //TxMidiByte(0x7E);    // 7E: Non-realtime universal SysEx ID
   //TxMidiByte(0x7F);    // 7F: Device ID (7F = all devices)
   //TxMidiByte(0x09);    // 09: Sub-ID 1 (General MIDI)
   //TxMidiByte(0x01);    // 01: Sub-ID 2 (GM Reset)
   //TxMidiByte(0xF7);    // F7: End of SysEx
   //
   //
   //TxMidiByte(0xF0);    // F0: Start of SysEx
   //TxMidiByte(0x41);    // 41: Roland manufacturer ID
   //TxMidiByte(0x00);    // 00: Device ID (may need adjustment for your setup)
   //TxMidiByte(0x1E);    // 42: Model ID (this is often used for early Roland devices)
   //TxMidiByte(0x12);    // 12: Command ID (Data Set)
   //TxMidiByte(0x40);    // 40 00 7F 00: Payload (example data; adjust if needed)
   //TxMidiByte(0x00);
   //TxMidiByte(0x7F);
   //TxMidiByte(0x00);
   //TxMidiByte(0x41);    // 41: Checksum (you can calculate it if you change the payload)
   //TxMidiByte(0xF7);    // F7: End of SysEx
   //
   //
   //TxMidiByte(0xF0); //F0: Start of SysEx   Doesn not work with the ROland s-330
   //TxMidiByte(0x41); //41: Roland manufacturer ID
   //TxMidiByte(0x00); //<DeviceID>: Typically 00; could vary (try 00 or 7F for global)
   //TxMidiByte(0x1E); //16: Model ID (for older Roland devices)
   //TxMidiByte(0x12); //12: Command ID (Data Set)
   //TxMidiByte(0x00); //00 00 00: Address (you can try different values, e.g., 00 10 00)
   //TxMidiByte(0x00);
   //TxMidiByte(0x00);
   //TxMidiByte(0x7F); //7F: Data (test value to trigger reset or refresh)
   //TxMidiByte(0xF7); //F7: End of SysEx
   //
   //
   //Brute force approach, send note OFF on all notes
   for( uint8_t note = 0x0C; note <= 0x78; note++)
   {
      SoundStop(note);
   }
   SoundStop(0x0C);
}
void SoundStart(uint8_t Note, uint8_t Velocity)
{
   TxMidiByte(0x90);       // Note ON
   TxMidiByte(Note);       // Note
   TxMidiByte(Velocity);   // Velocity
}
void SoundStop(uint8_t Note)
{
   TxMidiByte(0x80);       // Note OFF
   TxMidiByte(Note);       // Note
   TxMidiByte(40);         // Velocity (does not care)
}

void Tremolo(uint8_t Value)   // Value of tremolo 0-0x7F
{
   TxMidiByte(0xB0);
   TxMidiByte(0x01);
   TxMidiByte(Value);
}

// 0xE0 LSB MSB  (7 bits)
// MSB signed 
// 0x00 full left  0
// 0x40 middle     64
// 0x7F full right 127
void PitchBend(int8_t Value)   // Value of tremolo -64 to 63
{
   TxMidiByte(0xE0);
   TxMidiByte(0x00);
   TxMidiByte(Value+64);
}
