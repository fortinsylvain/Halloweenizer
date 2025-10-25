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
#include "type.h"
#include "dmx.h"
#include "midi.h"
#include "notes.h"
#include "scene.h"

#define Str_Version "October 25, 2025 11H13"

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

uint32_t MainLoopCount = 0;

//io_irq_ctrl_hw_t *irq_ctrl_base = &iobank0_hw->proc0_irq_ctrl;

// Declare prototype for function
void InterpretCmdString(char *);
void StoreInterpByte(uint8_t);

#define PacketSize 20
uint8_t Packet[PacketSize];

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
      busy_wait_ms(50);
      SetDmxLed(false);   // DMX LED OFF
      busy_wait_ms(50);
   }

   for(uint8_t i = 0; i < 8; i++)
   {
      SetMidiLed(true);   // MIDI LED ON
      busy_wait_ms(50);
      SetMidiLed(false);   // MIDI LED OFF
      busy_wait_ms(50);
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

   bool NoteExpire;
   uint64_t TimeNoteStartUs;

   // Loop forever
   while (true)
   {
      printf("----------\r\n");
      printf("Loop BEGIN\r\n");
      printf("----------\r\n");

      // Midi test
      MidiReset();

      // Dark choir scene
      scene_dark_choir();

      // White ramp up down
      scene_white_ramp_up_down(); 

      // Random Polyphonic scene
      scene_random_polyphonic();

      // Halloween orange scene
      scene_halloween_orange();

      // Automatic color sudent change
      scene_automatic_color_change();

      // Yellow candy treets scene
      scene_yellow_candy_treets();

      // Normal light scene
      scene_normal_light();

      // Defective light scene
      scene_defective_light();

      // Cold purple black light scene
      scene_cold_purple_black_light();

      // Red hearth beat scene
      scene_red_hearth_beat();

      // Wavy color green scen
      scene_wavy_color_green();

      // Defective light random glitch scene
      scene_defective_light_random_glitch();

      // Ramdom color quick change scene
      scene_random_color_quick_change();

      // Pale skin scene
      scene_pale_skin();
      
      // Cold blue scene
      scene_cold_blue();

      // Thunderstorm lightning effect scene
      scene_thunderstorm_lightning_effect();

      // Blue dark modulation scene
      scene_blue_dark_modulation();

      // White flash strobes on red background scene
      scene_white_flash_strobes_on_red_background();

      // Burning scene
      scene_burning();

      // Automatic strobe scene
      scene_automatic_strobe();

      // Automatic color gradual change
      scene_automatic_color_gradual_change();

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
      MidiReset();
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


