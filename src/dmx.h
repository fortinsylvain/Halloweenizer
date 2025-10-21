#include <stdint.h>  // needed for uint8_t
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "pins.h"
#include "led.h"

// --- DMX timing constants (in microseconds) ---
#define break_us 88
#define mab_us 8
#define start_us 4
#define data_us 4
#define stop_us 4 // there is two stop bit

// --- Function prototypes ---
void send_one_frame(uint8_t Data);
void send_packet(uint8_t* Data);



