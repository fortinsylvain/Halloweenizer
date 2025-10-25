
#include "dmx.h"

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

void Black(uint16_t time_ms)
{
   color_t colorA;
   colorA.intensity = 255;    // Black
   colorA.red = 0;
   colorA.green = 0;
   colorA.blue = 0;
   send_color(colorA, time_ms);
}

void send_color(color_t color, uint16_t time_ms)
{
   #define PacketSize 20
   uint8_t Packet[PacketSize];

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

void strobe(uint8_t number, uint16_t period_ms)
{
   #define PacketSize 20
   uint8_t Packet[PacketSize];

   uint8_t n = number;
   for (int n = 0; n < number; n++)
   {
      Packet[0] = 0;
      Packet[1] = 255; // intensity
      Packet[2] = 255; // Red
      Packet[3] = 255; // Green
      Packet[4] = 255; // Blue
      Packet[5] = 0;
      Packet[6] = 0; // Function selection
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

void ramp_color(color_t colorA, color_t colorB, uint16_t time_ms)
{
   #define PacketSize 20
   uint8_t Packet[PacketSize];

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

// White flash strobe using the automatic strober
// speed: 8 slowest, 255 fastest
void autostrobe(color_t color, uint8_t speed, uint16_t time_ms)
{
   #define PacketSize 20
   uint8_t Packet[PacketSize];

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

