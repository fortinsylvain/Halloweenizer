#ifndef NOTES_H
#define NOTES_H

#include <stdint.h>
#include <stddef.h>   // for size_t

// Declares the arrays as extern, so they can be used anywhere.

// Organ note array
extern uint8_t organ_notes[];
extern const size_t NUM_ORGAN_NOTES;

// Sound effect note array
extern uint8_t sound_effect_notes[];
extern const size_t NUM_SFX_NOTES;

#endif // NOTES_H

