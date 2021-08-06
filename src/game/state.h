#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "types.h"
#include "atlas.h"
#include "pak.h"
#include "game/cube.h"

struct game_state_t;

// Window state information
struct game_state_win_t {
  // Window size
  // Read by the game to determine window size
  u32 width, height;
};

// Renderer specific state data
struct game_state_render_t {
  const game_state_t *game;

  // Used to load atlases into renderer
  // Set to NULL when atlas is loaded
  const atlas_t *atlas[ATLAS_COUNT];
};

// Game state struct
static constexpr uptr STATE_MAXCUBES = 256;
struct game_state_t {
  game_state_win_t w; // Window state
  game_state_render_t r; // Render state

  vec4 pos;

  uptr cubeCnt;
  cube_t cubes[STATE_MAXCUBES];
  cube_t cube; // TEMP

  f32 fovy; // Vertical field of view

  f32 yaw; // Player camera yaw
  f32 pitch; // Player camera pitch
};

#endif //GAME_STATE_H
