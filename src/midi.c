#include "midi.h"

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

   Tremolo(0x00);  // Tremolo OFF
   PitchBend(0);  // Pitch Bend center
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
