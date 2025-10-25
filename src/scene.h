#ifndef SCENE_H
#define SCENE_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "midi.h"
#include "dmx.h"

void scene_dark_choir(void);
void scene_random_polyphonic(void);
void scene_halloween_orange(void);
void scene_automatic_color_change(void);
void scene_white_ramp_up_down(void);
void scene_yellow_candy_treets(void);
void scene_normal_light(void);
void scene_defective_light(void);
void scene_cold_purple_black_light(void);
void scene_red_hearth_beat(void);
void scene_wavy_color_green(void);
void scene_defective_light_random_glitch(void);
void scene_random_color_quick_change(void);
void scene_pale_skin(void);
void scene_cold_blue(void);
void scene_thunderstorm_lightning_effect(void);
void scene_blue_dark_modulation(void);
void scene_white_flash_strobes_on_red_background(void);
void scene_burning(void);
void scene_automatic_strobe(void);
void scene_automatic_color_gradual_change(void);

// effect functions
int GenerateRandomInt (int MaxValue);
int randomPitchBending(int center, int range);

#endif // SCENE_H

