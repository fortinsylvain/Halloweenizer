#include <stdint.h>
#include <stdbool.h>
#include "pins.h"
#include "led.h"
#include "hardware/gpio.h"

void SetMidiLed(bool state)
{
   if(state)
   {
      // MIDI LED ON
      gpio_set_dir(MIDI_LED, GPIO_OUT);   
      gpio_put(MIDI_LED, 0);   
   }
   else
   {
      // MIDI LED OFF
      gpio_set_dir(MIDI_LED, GPIO_IN); // High impedance
      //gpio_put(MIDI_LED, 1);   
      gpio_disable_pulls(MIDI_LED);    // Ensure no internal pull-up/down
   }
   
}
void SetDmxLed(bool state)
{
   if(state)
   {
      // DMX LED ON
      gpio_set_dir(DMX_LED, GPIO_OUT);
      gpio_put(DMX_LED, 0);   
   }
   else
   {
      // DMX LED OFF
      gpio_set_dir(DMX_LED, GPIO_IN);  // High impedance
      //gpio_put(DMX_LED, 1);   
      gpio_disable_pulls(DMX_LED);    // Ensure no internal pull-up/down
   }

}
