#include <stdint.h>  // needed for uint8_t
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>

#include "pins.h"
#include "led.h"

void TxMidiByte(uint8_t Data);
void MidiReset(void);
void SoundStart(uint8_t Note, uint8_t Velocity);
void SoundStop(uint8_t Note);
void Tremolo(uint8_t Value);   // Value of tremolo 0-0x7F
void PitchBend(int8_t Value);









