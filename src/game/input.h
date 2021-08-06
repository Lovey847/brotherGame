#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "types.h"
#include "key.h"

// Game input struct
struct game_input_t {
	// Keyboard input
	key_state_t k;

	// Mouse input
	i32 mx, my;

  FINLINE game_input_t() {
    mx = my = -1;
  }
};

#endif //GAME_INPUT_H
