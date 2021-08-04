#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "types.h"
#include "atlas.h"
#include "pak.h"

// Renderer specific state data
struct game_state_render_t {
  // Currently active atlases
  const atlas_t *atlas[ATLAS_COUNT];

  // Atlas pak entries
  pak_entry_t atlasEnt[ATLAS_COUNT];
};

// Game state struct
struct game_state_t {
  game_state_render_t r; // Render state

  f32 x, y, spd;
};

#endif //GAME_STATE_H
