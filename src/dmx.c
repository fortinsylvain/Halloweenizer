
#include "dmx.h"

// Generate one frame
// ---------------------------------------------------------------
// | START | D0 | D1 | D2 | D3 | D4 | D5 | D6 | D7 | STOP | STOP |
// ---------------------------------------------------------------
void send_one_frame(uint8_t Data)
{
   gpio_put(Data_GPIO, false); // START '0'
   busy_wait_us(start_us);
   
   (Data & 0x01) ? gpio_put(Data_GPIO, true) : gpio_put(Data_GPIO, false);  // D0
   busy_wait_us(data_us);

   (Data & 0x02) ? gpio_put(Data_GPIO, true) : gpio_put(Data_GPIO, false);  // D1
   busy_wait_us(data_us);

   (Data & 0x04) ? gpio_put(Data_GPIO, true) : gpio_put(Data_GPIO, false);  // D2
   busy_wait_us(data_us);

   (Data & 0x08) ? gpio_put(Data_GPIO, true) : gpio_put(Data_GPIO, false);  // D3
   busy_wait_us(data_us);
   
   (Data & 0x10) ? gpio_put(Data_GPIO, true) : gpio_put(Data_GPIO, false);  // D4
   busy_wait_us(data_us);

   (Data & 0x20) ? gpio_put(Data_GPIO, true) : gpio_put(Data_GPIO, false);  // D5
   busy_wait_us(data_us);

   (Data & 0x40) ? gpio_put(Data_GPIO, true) : gpio_put(Data_GPIO, false);  // D6
   busy_wait_us(data_us);

   (Data & 0x80) ? gpio_put(Data_GPIO, true) : gpio_put(Data_GPIO, false);  // D7
   busy_wait_us(data_us);

   gpio_put(Data_GPIO, true);   // STOP '1'
   busy_wait_us(stop_us);        // two bit time
   busy_wait_us(stop_us);
}

// MAB | data0 data1 data2 data3 data4 | BREAK
void send_packet(uint8_t* Data)
{
   SetDmxLed(true); // Led ON during transmission

   gpio_put(Data_GPIO, true); // MAB '1'
   busy_wait_us(mab_us);

   send_one_frame(Data[0]);
   send_one_frame(Data[1]);
   send_one_frame(Data[2]);
   send_one_frame(Data[3]);
   send_one_frame(Data[4]);
   send_one_frame(Data[5]);
   send_one_frame(Data[6]);
   send_one_frame(Data[7]);
   send_one_frame(Data[8]);
   send_one_frame(Data[9]);
   send_one_frame(Data[10]);
   send_one_frame(Data[11]);

   gpio_put(Data_GPIO, false); // BREAK '0'
   busy_wait_us(break_us);

   SetDmxLed(false); // Led OFF after transmission
}

