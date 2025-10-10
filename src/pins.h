// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define TP0_GPIO 6      // TP0 GP6 PCB pin<9>
#define TP1_GPIO 7      // TP1 GP7 PCB pin<10>  
#define TP2_GPIO 8      // TP2 GP8 PCB pin<11> 
#define TP3_GPIO 9      // TP3 GP9 PCB pin<12>
#define TP4_GPIO 10     // TP4 GP10 PCB pin<14>
#define TP5_GPIO 11     // TP5 GP11 PCB pin<15>
#define TP6_GPIO 12     // TP6 GP12 PCB pin<16>
#define TP7_GPIO 13     // TP7 GP13 PCB pin<17>   Main Loop
#define MidiOut_GPIO 14 // GP14 PCB pin<19>   Mido serial data out
#define Data_GPIO 15    // GP15 PCB pin<20>   Serial data for DMX512
#define DMX_LED 16      // GP16 PCB pin<21>   LED indicating tx on DMX512
#define MIDI_LED 17     // GP17 PCB pin<22>   LED indicating tx on MIDI

//N.C. More than 12 channels make sampling drop from 25 to 16 MS/s on Zaleae Logic 16 LA

#define LED_GPIO 25    // GP25 and don't have a pin on RP Pico board