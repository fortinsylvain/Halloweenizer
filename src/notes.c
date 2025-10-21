#include "notes.h"

// Note = 36;   // C2 Organ
// Note = 37;   // C#2 Organ
// Note = 38;   // D2 GLutural Water sink lower frequency pitch
// Note = 39;   // D#2 Organ
// Note = 40;   // E2 Organ
// Note = 41;   // F2 Organ
// Note = 42;   // F#2 GLutural Water sink frequency pitch
// Note = 43;   // G2 Laph Very lower frequency pitch
// Note = 44;   // G#2 Organ
// Note = 45;   // A2 Beast Glutural Lower Pitch
// Note = 46;   // A#2 Organ
// Note = 47;   // B2 Hurlement type syrene lower pitch (one shot)
// Note = 48;   // C3 Hurlement type syrene lower pitch (one shot)
// Note = 49;   // C#3 Organ
// Note = 50;   // D3 Laph Very low frequency pitch
// Note = 51;   // D#3 Beast Lower
// Note = 52;   // E3 Hurlement type syrene lower pitch (one shot)
// Note = 53;   // F3 Organ
// Note = 54;   // F#3 Hurlement type syrene (one shot)
// Note = 55;   // G3 Beast Glutural Low
// Note = 56;   // G#3 Organ
// Note = 57;   // A3 Thunderstorm (one shot)
// Note = 58;   // A#3 Organ
// Note = 59;   // B3 Organ
// Note = 60;   // C4 Rire lent (joue pendant l'appuie)
// Note = 61;   // C#4 Organ
// Note = 62;   // D4 Organ
// Note = 63;   // D#4 Monster Whaaa
// Note = 64;   // E4 Hurlement type syrene higher pitch (one shot)
// Note = 65;   // F4 Monster Whaaa higher pitch
// Note = 66;   // F#4 Organ
// Note = 67;   // G4 Organ
// Note = 68;   // G#4 Organ
// Note = 69;   // A Monster Classic
// Note = 70;   // A#4 Organ
// Note = 71;   // B4 Organ
// Note = 72;   // C5 Beast Glutural High
// Note = 73;   // C#5 Monster Whaaa higher pitch
// Note = 74;   // D5 Wild animal
// Note = 75;   // D#5 Gun shot (one shot)
// Note = 76;   // E5 Wild animal higher pitch
// Note = 77;   // F5 Crying body (one shot)
// Note = 78;   // F#5 Wild animal higher pitch
// Note = 79;   // G5 Crying body (one shot)
// Note = 80;   // G#5 Organ
// Note = 81;   // A5 Organ
// Note = 82;   // A#5 Laught High pitch
// Note = 83;   // B5 Wild animal higher pitch
// Note = 84;   // C6 Wild animal higher pitch
// Note = 85;   // C#6 -
// Note = 86;   // D6 Wild animal higher pitch
// Note = 87;   // D#6 Crying female (one shot)
// Note = 88;   // E6 Croincement court
// Note = 89;   // F6 Laught child
// Note = 90;   // F#6 Wild animal higher pitch
// Note = 91;   // G6 Croincement court 2
// Note = 92;   // G#6 Blody bird
// Note = 93;   // A6 Monster children Whaaa higher pitch
// Note = 94;   // A#6 Croincement court 3
// Note = 95;   // B6 Animal crying Highly

uint8_t organ_notes[] = {
    36, 37, 39, 40, 41, 44, 46, 49, 53, 56, 58, 59, 61, 62, 66, 67, 68, 70, 71, 80, 81};
const size_t NUM_ORGAN_NOTES = sizeof(organ_notes) / sizeof(organ_notes[0]);

uint8_t sound_effect_notes[] = {
    38, 42, 43, 45, 47, 48, 50, 51, 52, 54, 55, 57, 60, 63, 64, 65, 69, 72, 73, 74,
    75, 76, 77, 78, 79, 82, 83, 84, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95};
const size_t NUM_SFX_NOTES = sizeof(sound_effect_notes) / sizeof(sound_effect_notes[0]);
