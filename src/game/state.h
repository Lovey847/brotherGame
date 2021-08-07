#ifndef GAME_STATE_H
#define GAME_STATE_H

#define GAME_STATE_EDITOR

#include "types.h"
#include "atlas.h"
#include "pak.h"
#include "game/map.h"

#ifdef GAME_STATE_EDITOR

// Map properties
struct game_state_map_prop_t {
  // Map filename
  const char *filename;

  // Map string hash name
  // 0 to load no map
  str_hash_t hashName;

  // Previous and next map string hashes
  str_hash_t prev, next;

  // Map atlas
  str_hash_t atlas;

  // Atlas image names
  const str_hash_t imgNames[64];

  // Atlas image count
  uptr imgCount;
};

static constexpr uptr GAME_STATE_MAPCOUNT = 1;

static const game_state_map_prop_t game_state_maps[GAME_STATE_MAPCOUNT] = {
  {
    "../gen/data/files/maps/000.map",
    str_hash("maps/000.map"),
    0, str_hash("maps/001.map"),
    str_hash("atlases/000.atl"),
    {
      str_hash("gray"),
      str_hash("tutorial"),
      str_hash("wood"),
    },
    3,
  },
};

#endif

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

  // Set to true when renderer
  // should load level assets
  ubool load;

  // Used to load atlases into renderer
  const atlas_t *atlas[ATLAS_COUNT];
};

// Game state struct
static constexpr uptr STATE_MAXCUBES = 256;
struct game_state_t {
  game_state_win_t w; // Window state
  game_state_render_t r; // Render state

  // Game map
  map_t map;

  vec4 pos;

  f32 fovy; // Vertical field of view

  f32 yaw; // Player camera yaw
  f32 pitch; // Player camera pitch

  // Map editor state
#ifdef GAME_STATE_EDITOR
  // Size of placeable cube
  vec4 blockSize;

  // Current cube grid, power of 2
  // w is always 1.f
  vec4 blockGrid;

  // Current cube texture, index into game_state_texNames
  uptr blockTexture;

  // Block distance from camera
  f32 blockDist;

  // Buffer of cubes ready to be written out
  map_cube_t editorCubes[256];
  uptr editorCubeCount;

  // Loading zone to previous map
  map_cube_t prevLoad;

  // Loading zone to next map
  map_cube_t nextLoad;
#endif
};

#endif //GAME_STATE_H
