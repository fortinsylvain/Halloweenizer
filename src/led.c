#include <stdint.h>
#include <stdbool.h>
#include "pins.h"
#include "led.h"
#include "hardware/gpio.h"

void SetMidiLed(bool state)
{
   if(state)
   {
      gpio_set_dir(MIDI_LED, GPIO_OUT);
      gpio_put(MIDI_LED, 0);   // MIDI LED ON
   }
   else
   {
      gpio_set_dir(MIDI_LED, GPIO_IN);
      gpio_put(MIDI_LED, 1);   // MIDI LED OFF
   }
   
}
void SetDmxLed(bool state)
{
   if(state)
   {
      gpio_set_dir(DMX_LED, GPIO_OUT);
      gpio_put(DMX_LED, 0);   // DMX LED ON
   }
   else
   {
      gpio_set_dir(DMX_LED, GPIO_IN);
      gpio_put(DMX_LED, 1);   // DMX LED OFF
   }

}
