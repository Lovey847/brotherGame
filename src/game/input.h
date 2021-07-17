#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "types.h"
#include "key.h"

enum game_input_mbtn_t : uptr {
	MC_PRIMARY = 0x79,
	MC_MIDDLE = 0x7a,
	MC_SECONDARY = 0x7b,

	MC_SCROLLUP = 0x7c,
	MC_SCROLLDOWN = 0x7d
};

// Game input struct
struct game_input_t {
	// Keyboard input
	key_state_t k;

	// Mouse input
	f32 mx, my; // Normalized mouse coordinates

	// Mouse buttons are treated as keyboard keys
	FINLINE ubool mbtn(game_input_mbtn_t id) const {return k.down[(uptr)id];}
	FINLINE ubool mbtnPressed(game_input_mbtn_t id) const {return k.pressed[(uptr)id];}
	FINLINE ubool mbtnReleased(game_input_mbtn_t id) const {return k.released[(uptr)id];}
};

#endif //GAME_INPUT_H
