#include "scene.h"
#include "notes.h"        // for organ_notes and NUM_ORGAN_NOTES
#include "pico/stdlib.h"  // for time_us_64(), sleep_ms(), etc.
#include <stdlib.h>
#include <time.h>
#include <stdio.h>        // for printf()

extern uint8_t Packet[8];  // LED or color control packet

void scene_dark_choir(void)
{
   printf("Play Dark Choir Scene\r\n");

   color_t colorA;
   colorA.intensity = 255;
   colorA.red = 10;
   colorA.green = 10;
   colorA.blue = 255;

   int phraseQty = 4;

   // Pattern to repeat: -64 → 0 → 63 → 0 → -64
   const int phrase1[] = {-25, -59, -24, 6, -30, -64, -32};
   const int phrase2[] = {6, -29, -3, 28, 3, -25, -6};
   const int phrase3[] = {26, 48, 28, 7, -28, -49, -22};
   const int phrase4[] = {-34, -64, -37, -8, -38, -57, -36};

   const int numSteps = sizeof(phrase1) / sizeof(phrase1[0]);
   int step = 0;

   for (int iPhrase = 0; iPhrase < phraseQty; iPhrase++)
   {
      const int *phrase = NULL;
      switch (iPhrase)
      {
      case 0:
         phrase = phrase1;
         break;
      case 1:
         phrase = phrase2;
         break;
      case 2:
         phrase = phrase3;
         break;
      case 3:
         phrase = phrase4;
         break;
      }

      #define NUM_NOTES_TO_PLAY 3
      uint8_t playingNotes[NUM_NOTES_TO_PLAY] = {37, 40, 47};
      // Play the notes
      for (int i = 0; i < NUM_NOTES_TO_PLAY; i++)
      {
         SoundStart(playingNotes[i], 100); // medium velocity
      }

      // Perform pitch bending sequence
      for (int i = 0; i < numSteps; i++)
      {
         int target = phrase[i];
         int variation;

         // Smaller wobble near 0, larger at extremes
         if (target == 0)
            variation = 3;
         else
            variation = 8;

         int detuned = randomPitchBending(target, variation);
         PitchBend((int8_t)detuned);
         send_color(colorA, 1000); // contain the interval between bend
      }

      // Stop previous notes
      for (int i = 0; i < NUM_NOTES_TO_PLAY; i++)
      {
         SoundStop(playingNotes[i]);
      }
      Black(1000); // fade to black for end of phrase
      
   }

   Black(1000); // fade to black
}

// *********************************************************
// Random Polyphonic With Color Changes scene
// *********************************************************
void scene_random_polyphonic(void)
{
   printf("Play Random Polyphonic With Color Changes\r\n");

   bool Timereach;
   bool NoteExpire;
   uint64_t TimeStartUs;
   uint64_t TimeNoteStartUs;
   uint64_t delta_t_us;

   Timereach = false;
   TimeStartUs = time_us_64();
   uint32_t time_ms = 20000; // duration

   Packet[7] = 0;
   Packet[6] = 200; // Color function code

   // Seed random once
   static bool seeded = false;
   if (!seeded)
   {
      srand(time(NULL));
      seeded = true;
   }

   uint8_t playingNotes[4];
   uint8_t numNotesToPlay;

   while (!Timereach)
   {
      // Pick new random notes and play them
      numNotesToPlay = 2 + rand() % 2; // 2–3 notes
      for (int i = 0; i < numNotesToPlay; i++)
      {
         playingNotes[i] = organ_notes[rand() % NUM_ORGAN_NOTES];
         SoundStart(playingNotes[i], 64); // medium velocity
      }

      // Randomize color for each packet
      Packet[0] = 0; // Start code
      Packet[1] = rand() % 128; // Intensity
      Packet[2] = rand() % 256; // Red
      Packet[3] = rand() % 256; // Green
      Packet[4] = rand() % 256; // Blue

      // Wait until notes expire (~1 second)
      NoteExpire = false;
      TimeNoteStartUs = time_us_64();
      while (!NoteExpire)
      {
         send_packet(Packet);
         if (time_us_64() - TimeNoteStartUs > 1000000)
            NoteExpire = true;
      }

      // Stop previous notes
      for (int i = 0; i < numNotesToPlay; i++)
         SoundStop(playingNotes[i]);

      // Check total scene time
      delta_t_us = time_us_64() - TimeStartUs;
      if (delta_t_us > (time_ms * 1000))
         Timereach = true;
   }

   // Stop any lingering notes and fade out
   for (int i = 0; i < numNotesToPlay; i++)
      SoundStop(playingNotes[i]);

   Black(2000); // fade to black
}

// *********************************************************
// Random Polyphonic With Color Changes scene
// *********************************************************
void scene_halloween_orange(void)
{
   // Halloween orange
   printf("Halloween orange\r\n");

   uint8_t Note;
   color_t colorA, colorB, colorC;
   bool Timereach = false;
   uint64_t TimeStartUs;
   uint64_t delta_t_us;
   uint16_t time_ms;


   Note = 47; // B2 Hurlement type syrene lower pitch (one shot)
   SoundStart(Note, 127);
   colorA.intensity = 255;
   colorA.red = 0;
   colorA.green = 0;
   colorA.blue = 0;
   colorB.intensity = 255;
   colorB.red = 255;
   colorB.green = 23;
   colorB.blue = 2;
   ramp_color(colorA, colorB, 4000);
   // send_color(colorB, 13000);
   //  play organ for a while
   // uint8_t my_note_array[] = {36, 37, 39, 40, 41, 44, 46, 49, 53, 56, 58, 59, 61, 62, 66, 67, 68, 70, 71, 80, 81};
   Timereach = false;
   TimeStartUs = time_us_64(); // Time snapshot
   time_ms = 20000;
   Tremolo(0x7F);             // Max
   while (Timereach == false) // keep sending packets as long as not reached time period?
   {
      delta_t_us = time_us_64() - TimeStartUs;
      uint8_t OrganRandomNumber = GenerateRandomInt(20);
      // uint8_t OrganRandomNote = my_note_array[OrganRandomNumber];
      uint8_t OrganRandomNote = organ_notes[OrganRandomNumber];
      uint16_t OrganRandomDuration = GenerateRandomInt(1500);
      uint8_t OrganRandomVelocity = GenerateRandomInt(127);
      SoundStart(OrganRandomNote, OrganRandomVelocity);
      // busy_wait_ms(300);
      send_color(colorB, OrganRandomDuration);
      SoundStop(OrganRandomNote);
      if (delta_t_us > (time_ms * 1000))
      {
         Timereach = true;
      }
   }
   Tremolo(0); // OFF
   SoundStop(Note);
   SoundStart(Note, 127);
   ramp_color(colorB, colorA, 4000);
   SoundStop(Note);
   Black(2000);
}

// *********************************************************
// Automatic color sudent change
// *********************************************************
void scene_automatic_color_change(void)
{
   // Automatic color sudent change
   printf("Automatic color change\r\n");

   uint8_t Note;
   color_t colorA, colorB, colorC;
   bool Timereach;
   uint64_t TimeStartUs;
   uint64_t delta_t_us;
   uint16_t time_ms;
   

   SoundStart(43, 127); // G2 Laph Very lower frequency pitch
   SoundStart(77, 127); // F5 Crying body (one shot)
   Timereach = false;
   TimeStartUs = time_us_64(); // Time snapshot
   time_ms = 20000;
   Packet[0] = 0;
   Packet[1] = 0; // intensity
   Packet[2] = 0; // red
   Packet[3] = 0; // green
   Packet[4] = 0; // blue
   Packet[5] = 0;
   Packet[6] = 200;           // Function selection
                              // 254, 255 sound control
                              // 200 Color change
                              // 100, 151 Color change gradually
                              // 20, 39, 51 Turn on
                              // 0, 1, 10 Strobe
   while (Timereach == false) // keep sending packets as long as not reached time period?
   {
      delta_t_us = time_us_64() - TimeStartUs;
      send_packet(Packet);
      if (delta_t_us > (time_ms * 1000))
      {
         Timereach = true;
      }
   }
   SoundStop(43);
   SoundStop(77);
   Black(2000);
}

void scene_white_ramp_up_down(void)
{
   SoundStart(43, 127); // G2 Laph Very lower frequency pitch

   // white ramp up down
   printf("White ramp up down\r\n");

   color_t colorA, colorB, colorC;

   colorA.intensity = 255;
   colorA.red = 0;
   colorA.green = 0;
   colorA.blue = 0;
   colorB.intensity = 255;
   colorB.red = 255;
   colorB.green = 255;
   colorB.blue = 255;
   ramp_color(colorA, colorB, 1000); // ramp up
   ramp_color(colorB, colorA, 2000); // ramp down
   busy_wait_ms(2000);
   ramp_color(colorA, colorB, 800);  // ramp up
   ramp_color(colorB, colorA, 3000); // ramp down
   busy_wait_ms(2000);
   ramp_color(colorA, colorB, 700);  // ramp up
   ramp_color(colorB, colorA, 4000); // ramp down
   busy_wait_ms(2000);

   SoundStop(43);
}

// *********************************************************
// Yellow candy treets scene
// *********************************************************
void scene_yellow_candy_treets(void)
{
   // yellow candy treets
   printf("Yellow candy treets\r\n");

   color_t colorA, colorB;

   SoundStart(79, 127); // G5 Crying body (one shot)
   busy_wait_ms(1000);
   SoundStop(79);
   colorA.intensity = 255; // Yellow
   colorA.red = 255;
   colorA.green = 99;
   colorA.blue = 0;
   colorB.intensity = 255; // Black
   colorB.red = 0;
   colorB.green = 0;
   colorB.blue = 0;
   send_color(colorB, 1);
   ramp_color(colorB, colorA, 4000);
   SoundStart(77, 127); // F5 Crying body (one shot)
   send_color(colorA, 13000);
   SoundStop(77);
   ramp_color(colorA, colorB, 4000);
   SoundStart(79, 127); // G5 Crying body (one shot)
   busy_wait_ms(1000);
   SoundStart(79, 127); // G5 Crying body (one shot)
   Black(2000);
   SoundStop(79);
   SoundStop(77);
}

// *********************************************************
// scene_Normal_light
// *********************************************************
void scene_normal_light(void)
{
   // Normal light
   printf("Normal light\r\n");

   color_t colorA;

   SoundStart(48, 70); // C3 Hurlement type syrene lower pitch (one shot)
   busy_wait_ms(1000);
   SoundStart(36, 100); // C2 Organ
   Tremolo(0x7F);       // Max tremolo
   colorA.intensity = 255;
   colorA.red = 255;
   colorA.green = 255;
   colorA.blue = 255;
   send_color(colorA, 10000);
   SoundStop(36);
   SoundStop(48);
   Tremolo(0); // tremolo stop
}

// *********************************************************
// scene_Defective_light
// *********************************************************
void scene_defective_light(void)
{
   // Defective light (black glitch)
   printf("Defective light (black glitch)\r\n");

   color_t colorA, colorB;

   SoundStart(50, 70); // D3 Laph Very low frequency pitch
   busy_wait_ms(1000);
   SoundStart(36, 100); // C2 Organ
   Tremolo(0x7F);       // Max tremolo
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
      int RandomPitchBend = (GenerateRandomInt(16) * 4 - 1) - 64;
      PitchBend(RandomPitchBend);
      send_color(colorA, GenerateRandomInt(3000));   // random ON time
      for (int n = 0; n < GenerateRandomInt(7); n++) // random number of glitch
      {
         send_color(colorB, GenerateRandomInt(100)); // random OFF time
         send_color(colorA, GenerateRandomInt(150)); // random ON time
      }
   }
   SoundStop(50);
   SoundStop(36);
   PitchBend(0);
   Tremolo(0); // tremolo stop
}

// *********************************************************
// Cold purple black light
// *********************************************************
void scene_cold_purple_black_light(void)
{
   printf("Cold purple black light\r\n");

   color_t colorA, colorB; 

   SoundStart(63, 100); // D#4 Monster Whaaa
   busy_wait_ms(2000);
   SoundStop(63);
   SoundStart(64, 100); // E4 Hurlement type syrene higher pitch (one shot)
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
}

// *********************************************************
// Red hearth beat
// *********************************************************
void scene_red_hearth_beat(void)
{
   printf("Red hearth beat\r\n");

   color_t colorA, colorB;

   SoundStart(60, 127); // C4 Rire lent (joue pendant l'appuie)
   busy_wait_ms(1000);
   SoundStop(60);
   SoundStart(43, 127); // G2 Laph Very lower frequency pitch
   colorA.intensity = 255;
   colorA.red = 0;
   colorA.green = 0;
   colorA.blue = 0;
   colorB.intensity = 255;
   colorB.red = 255;
   colorB.green = 0;
   colorB.blue = 0;
   for (int n = 0; n < 10; n++)
   {
      ramp_color(colorA, colorB, 100); // ramp up
      ramp_color(colorB, colorA, 90);  // ramp down
      busy_wait_ms(40);
      ramp_color(colorA, colorB, 130); // ramp up
      ramp_color(colorB, colorA, 140); // ramp down
      busy_wait_ms(800);
   }
   SoundStop(43);
}

// *********************************************************
// Wavy color green
// *********************************************************
void scene_wavy_color_green(void)
{
   printf("Wavy color green\r\n");

   color_t colorA, colorC;
   uint64_t delta_t_us;
   uint16_t time_ms;
   bool Timereach;
   uint64_t TimeStartUs;
   uint8_t Note;

   Note = 60; // C4 Rire lent (joue pendant l'appuie)
   SoundStart(Note, 90);
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
   float FrequencyIni = 0.75; // make the frequency change uop and down as time goes
   float Frequency;
   uint16_t Amplitude = 127;
   Note = 82; // A#5 Laught High pitch
   SoundStart(Note, 90);
   while (Timereach == false) // keep sending packets as long as not reached time period?
   {
      delta_t_us = time_us_64() - TimeStartUs;
      Frequency = FrequencyIni + FrequencyIni * sin((2 * 3.1416 * 0.05 * (float)delta_t_us) / 1000000);
      colorX.intensity = colorA.intensity;
      colorX.red = colorA.red;
      colorX.green = (uint8_t)(colorA.green + Amplitude * sin((2 * 3.1416 * Frequency * (float)delta_t_us) / 1000000));
      colorX.blue = colorA.blue;
      Packet[0] = 0;
      Packet[1] = colorX.intensity;
      Packet[2] = colorX.red;
      Packet[3] = colorX.green;
      Packet[4] = colorX.blue;
      Packet[5] = 0;
      Packet[6] = 0; // Function selection
                     // 254, 255 sound control
                     // 200 Color change
                     // 100, 151 Color change gradually
                     // 20, 39, 51 Turn on
                     // 0, 1, 10 Strobe
      send_packet(Packet);
      if (delta_t_us > (time_ms * 1000))
      {
         Timereach = true;
      }
   }
   SoundStop(Note);
   Note = 60; // C4 Rire lent (joue pendant l'appuie)
   SoundStart(Note, 90);
   Black(2000);
   SoundStop(Note);
}

// *********************************************************
// Defective light (random color glitch)
// *********************************************************
void scene_defective_light_random_glitch(void)
{
   printf("Defective light (random color glitch)\r\n");

   color_t colorA, colorB;
   uint8_t Note;
   
   Note = 54; // F#3 Hurlement type syrene (one shot)
   SoundStart(Note, 127);
   busy_wait_ms(50);
   SoundStop(Note);
   colorA.intensity = 255;
   colorA.red = 255; // normal white light color
   colorA.green = 255;
   colorA.blue = 255;
   for (int iter = 0; iter < 15; iter++)
   {
      send_color(colorA, GenerateRandomInt(3000)); // random ON time
      Note = 55;                                   // G3 Beast Glutural Low
      SoundStart(Note, 127);
      for (int n = 0; n < GenerateRandomInt(7); n++) // random number of glitch
      {
         colorB.red = GenerateRandomInt(255); // paranormal random color
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
}

// *********************************************************
// Random color quick change
// *********************************************************
void scene_random_color_quick_change(void)
{
   printf("Random color quick change\r\n");

   color_t colorA;
   uint8_t my_randomColorSound_array[] = {90, 91, 93, 95};
   for (int iter = 0; iter < 8; iter++)
   {
      uint8_t Number_Of_Color_Flash = GenerateRandomInt(25);
      uint8_t randomColorSound = my_randomColorSound_array[GenerateRandomInt(3)];
      SoundStart(randomColorSound, 60);
      for (int n = 0; n < Number_Of_Color_Flash; n++)
      {
         colorA.intensity = GenerateRandomInt(255);
         colorA.red = GenerateRandomInt(255);
         colorA.green = GenerateRandomInt(255);
         colorA.blue = GenerateRandomInt(255);
         send_color(colorA, GenerateRandomInt(200)); // Red
      }
      SoundStop(randomColorSound);
      colorA.intensity = 255;
      colorA.red = 0;
      colorA.green = 0;
      colorA.blue = 0;
      send_color(colorA, GenerateRandomInt(4000)); // Black for random duration
   }
}

// *********************************************************
// Pale skin scene
// *********************************************************
void scene_pale_skin(void)
{
   // Pale skin
   printf("Pale skin\r\n");

   uint8_t Note;
   color_t colorA, colorB, colorC;

   Note = 38; // D2 GLutural Water sink lower frequency pitch
   SoundStart(Note, 60);
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
   Note = 42; // F#2 GLutural Water sink frequency pitch
   SoundStart(Note, 100);
   ramp_color(colorB, colorC, 4000);
   // send_color(colorB, 4000);
   SoundStop(Note);
   Note = 38; // D2 GLutural Water sink lower frequency pitch
   SoundStart(Note, 90);
   ramp_color(colorC, colorB, 4000);
   SoundStop(Note);
   Note = 42; // F#2 GLutural Water sink frequency pitch
   SoundStart(Note, 127);
   ramp_color(colorB, colorC, 4000);
   SoundStop(Note);
   Note = 38; // D2 GLutural Water sink lower frequency pitch
   SoundStart(Note, 90);
   ramp_color(colorC, colorB, 4000);
   SoundStop(Note);
   Note = 42; // F#2 GLutural Water sink frequency pitch
   SoundStart(Note, 50);
   ramp_color(colorB, colorA, 4000);
   SoundStop(Note);
   Black(2000);
}

// *********************************************************
// Cold blue scene
// *********************************************************
void scene_cold_blue(void)
{
   printf("Cold blue\r\n");
   color_t colorA, colorB;
   uint8_t Note;

   Note = 43; // G2 Laph Very lower frequency pitch
   SoundStart(Note, 127);
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
}

// *********************************************************
// Thunderstorm lightning effect using white flash strobes
// *********************************************************
void scene_thunderstorm_lightning_effect(void)
{
   printf("Thunderstorm lightning effect\r\n");

   color_t colorA, colorB;
   uint8_t Note;

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
   Note = 57; // A3 Thunderstorm (one shot)
   for (int iter = 0; iter < 12; iter++)
   {
      uint8_t Number_Of_Flash = (uint8_t)GenerateRandomInt(ThunderMaxNumberFlash - ThunderMinNumberFlash) + ThunderMinNumberFlash;
      uint16_t ThunderFlashPeriod = GenerateRandomInt(ThunderMaxPeriod_ms - ThunderMinPeriod_ms) + ThunderMinPeriod_ms;
      uint8_t ThunderSoundIntensity = (uint8_t)(127 * Number_Of_Flash / ThunderMaxNumberFlash);
      SoundStart(Note, ThunderSoundIntensity);
      SoundStop(Note);
      strobe(Number_Of_Flash, ThunderFlashPeriod);
      // busy_wait_ms(GenerateRandomInt(4000));
      send_color(colorB, 2000 + GenerateRandomInt(4000));
   }
   ramp_color(colorB, colorA, 5000);
}

// *********************************************************
// Blue dark modulation
// *********************************************************
void scene_blue_dark_modulation(void)
{
   printf("Blue dark modulation\r\n");

   color_t colorA, colorB;
   uint8_t Note;

   colorA.intensity = 255;
   colorA.red = 0;
   colorA.green = 0;
   colorA.blue = 0;
   colorB.intensity = 255;
   colorB.red = 0;
   colorB.green = 0;
   colorB.blue = 255;
   ramp_color(colorA, colorB, 5000); // slow ramp up
   colorA.blue = 40;
   for (int n = 0; n < 7; n++)
   {
      uint8_t BlueSoundIntensity = (uint8_t)120 * (n + 1) / 8;
      SoundStart(74, BlueSoundIntensity);      // D5 Wild animal
      ramp_color(colorB, colorA, 2000);        // half fade
      SoundStart(76, BlueSoundIntensity + 10); // E5 Wild animal higher pitch
      ramp_color(colorA, colorB, 2000);        // back to blue
      SoundStop(74);
      SoundStop(76);
   }
   colorA.blue = 0;
   ramp_color(colorB, colorA, 5000); // fade out
}

// *********************************************************
// White flash strobes on red background
// *********************************************************
void scene_white_flash_strobes_on_red_background(void)
{
   printf("White flash strobes on red background\r\n");
   
   color_t colorA, colorB, colorC;
   uint8_t Note;

   Note = 43; // G2 Laph Very lower frequency pitch
   SoundStart(Note, 127);
   colorA.intensity = 255; // Red background
   colorA.red = 255;
   colorA.green = 0;
   colorA.blue = 0;
   send_color(colorA, 1000); // White
   colorB.intensity = 255;
   colorB.red = 255;
   colorB.green = 255;
   colorB.blue = 255;
   colorC.intensity = 255; // Black
   colorC.red = 0;
   colorC.green = 0;
   colorC.blue = 0;
   for (int iter = 0; iter < 8; iter++)
   {
      SoundStart(77, 40);     // F5 Crying body (one shot)
      send_color(colorB, 60); // White strobe
      // send_color(colorC, 100);   // Black
      // send_color(colorA, 700);  // Red
      ramp_color(colorC, colorA, 200);
      // strobe(3, 150);
      send_color(colorA, 1000); // Red
   }
   SoundStop(77);
   SoundStop(Note);
   Black(2000);
}

// *********************************************************
// Burning scene
// *********************************************************
void scene_burning(void)
{
   // burning
   printf("Burning\r\n");

   color_t colorA, colorB, colorC;
   uint8_t Note;
   uint64_t delta_t_us;
   uint16_t time_ms;
   bool Timereach;
   uint64_t TimeStartUs;

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
   ramp_color(colorA, colorB, 3000); // Initial ramp up
   Note = 77;                        // F5 Crying body (one shot)
   SoundStart(Note, 127);
   SoundStop(Note);
   Note = 38; // D2 GLutural Water sink lower frequency pitch
   SoundStart(Note, 127);
   delta_t_us;
   Timereach = false;
   TimeStartUs = time_us_64(); // Time snapshot
   time_ms = 20000;
   while (Timereach == false) // keep sending packets as long as not reached time period?
   {
      delta_t_us = time_us_64() - TimeStartUs;

      ramp_color(colorB, colorC, GenerateRandomInt(400));
      ramp_color(colorC, colorB, GenerateRandomInt(400));

      if (delta_t_us > (time_ms * 1000))
      {
         Timereach = true;
      }
   }
   Note = 75; // D#5 Gun shot (one shot)
   SoundStart(Note, 100);
   SoundStop(Note);
   ramp_color(colorB, colorA, 3000); // Ramp down
   Black(3000);
}

// *********************************************************
// Automatic strobe scene
// *********************************************************
void scene_automatic_strobe(void)
{
   // Automatic strobe
   printf("Automatic strobe\r\n");
   
   color_t colorA;
   uint8_t Note;

   // void autostrobe(color_t color, uint8_t speed, uint16_t time_ms);
   colorA.intensity = 255; // Blue
   colorA.red = 0;
   colorA.green = 0;
   colorA.blue = 255;
   Note = 36; // C2 Organ
   SoundStart(Note, 80);
   autostrobe(colorA, 100, 3000);
   SoundStop(Note);
   Black(2000);
   colorA.intensity = 255; // Red
   colorA.red = 255;
   colorA.green = 0;
   colorA.blue = 0;
   Note = 37; // C#2 Organ
   SoundStart(Note, 80);
   autostrobe(colorA, 255, 1000);
   SoundStop(Note);
   Black(2000);
   colorA.intensity = 255; // Green
   colorA.red = 0;
   colorA.green = 255;
   colorA.blue = 0;
   Note = 40; // E2 Organ
   SoundStart(Note, 80);
   autostrobe(colorA, 140, 2000);
   SoundStop(Note);
   Black(1500);
   colorA.intensity = 255; // Red + Blue
   colorA.red = 255;
   colorA.green = 0;
   colorA.blue = 255;
   Note = 41; // F2 Organ
   SoundStart(Note, 80);
   autostrobe(colorA, 170, 1000);
   SoundStop(Note);
   Black(2200);
   colorA.intensity = 255; // Red + Green
   colorA.red = 255;
   colorA.green = 255;
   colorA.blue = 0;
   Note = 44; // G#2 Organ
   SoundStart(Note, 80);
   autostrobe(colorA, 190, 1000);
   SoundStop(Note);
   Black(1500);
   colorA.intensity = 255; // Green + Blue
   colorA.red = 0;
   colorA.green = 255;
   colorA.blue = 255;
   Note = 46; // A#2 Organ
   SoundStart(Note, 80);
   autostrobe(colorA, 210, 1000);
   SoundStop(Note);
   Black(2500);
   colorA.intensity = 255; // White
   colorA.red = 255;
   colorA.green = 255;
   colorA.blue = 255;
   Note = 49;          // C#3 Organ
   SoundStart(41, 80); // F2 Organ
   SoundStart(59, 80); // B3 Organ
   SoundStart(62, 80); // D4 Organ
   SoundStart(70, 80); // A#4 Organ
   autostrobe(colorA, 250, 2500);
   SoundStop(41);
   SoundStop(59);
   SoundStop(62);
   SoundStop(70);
   Black(3000);
}

// *********************************************************
// Automatic color gradual change
// *********************************************************
void scene_automatic_color_gradual_change(void)
{
   // Automatic color gradual change
   printf("Automatic color gradual change\r\n");

   color_t colorA, colorB, colorC;
   uint8_t Note;
   bool Timereach;
   uint64_t TimeStartUs;
   uint64_t delta_t_us;
   uint16_t time_ms;

   Timereach = false;
   TimeStartUs = time_us_64(); // Time snapshot
   time_ms = 20000;
   Packet[0] = 0;
   Packet[1] = 0; // intensity
   Packet[2] = 0; // red
   Packet[3] = 0; // green
   Packet[4] = 0; // blue
   Packet[5] = 0;
   Packet[6] = 100;           // Function selection
                              // 254, 255 sound control
                              // 200 Color change
                              // 100, 151 Color change gradually
                              // 20, 39, 51 Turn on
                              // 0, 1, 10 Strobe
   while (Timereach == false) // keep sending packets as long as not reached time period?
   {
      delta_t_us = time_us_64() - TimeStartUs;
      send_packet(Packet);
      if (delta_t_us > (time_ms * 1000))
      {
         Timereach = true;
      }
   }
   Black(3000);
}

//    Random value will be from 0 to #
int GenerateRandomInt(int MaxValue)
{
   unsigned int iseed = (unsigned int)time(NULL); // Seed srand() using time() otherwise it will start from a default value of 1
   // srand (iseed);
   int random_value = (int)((1.0 + MaxValue) * rand() / (RAND_MAX + 1.0)); // Scale rand()'s return value against RAND_MAX using doubles instead of a pure modulus to have a more distributed result.
   return (random_value);
}

// Return a random pitch bend value around a center point,
// clamped within [-64, 63]
int randomPitchBending(int center, int range)
{
   int val = center + (rand() % (2 * range + 1)) - range;
   if (val > 63)  val = 63;
   if (val < -64) val = -64;
   return val;
}

