#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "types.h"
#include "atlas.h"
#include "pak.h"

struct game_state_t;

// Window state information
struct game_state_win_t {
  // Window size
  // Usually read by the game, if set by the game
  // then the window module should resize the window
  // accordingly, or set the size back if it is unable
  // to resize the window
  u32 width, height;
  ubool fullscreen;
};

// Renderer specific state data
struct game_state_render_t {
  const game_state_t *game;

  // Used to load atlas into renderer
  // Set to NULL when renderer loads atlas
  const atlas_t *atlas[ATLAS_COUNT];
};

// Game state struct
struct game_state_t {
  game_state_win_t w; // Window state
  game_state_render_t r; // Render state

  f32 x, y, spd;

  // Atlas pak entries
  // Used to unmap the atlas entries, initialized to PAK_INVALID_ENTRY
  pak_entry_t atlasEnt[ATLAS_COUNT];
};

#endif //GAME_STATE_H
