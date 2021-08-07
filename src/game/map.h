#ifndef GAME_MAP_H
#define GAME_MAP_H

#include "types.h"
#include "endianUtil.h"
#include "str.h"
#include "mem.h"

// Cube in game map file
struct map_file_cube_t {
  endian_vec4 min;
  endian_vec4 max;

  union {
    str_hash_t img; // Image from level atlas
    str_hash_t map; // Map pak entry name (for loading zones)
  };
};

// Game map file
static constexpr u32 MAP_MAGIC = util_magic('M', 'A', 'P', 'F');
struct map_file_t {
  u32 magic; // == MAP_MAGIC

  endian_u32 cubeCount;

  map_file_cube_t prevLoad; // Previous map loading zone, invisible
  map_file_cube_t nextLoad; // Next map loading zone, invisible

  // Level atlas to load
  str_hash_t levelAtlas;

  // Map cubes
  map_file_cube_t cubes[1 /* cubeCount */];
};

// Used as bounding boxes, also drawn to the screen
struct map_cube_t {
  vec4 min, max;

  union {
    str_hash_t img; // Image from level atlas
    str_hash_t map; // Map pak entry name (for loading zones)
  };

  FINLINE map_cube_t() {}
  FINLINE map_cube_t(const map_file_cube_t &other) :
    min(other.min.v()), max(other.max.v()), img(other.img) {}
};

// Game map
struct map_t {
  map_cube_t prevLoad;
  map_cube_t nextLoad;

  // Map cubes
  map_cube_t *cubes = NULL;
  uptr cubeCount;

  str_hash_t levelAtlas; // Name of level atlas

  // Load map from file
  void load(mem_t &m, const map_file_t &f);

  // Free map data
  FINLINE void free(mem_t &m) {
    if (cubes) m.free(cubes);
  }
};

#endif //GAME_MAP_H
