#include "types.h"
#include "map.h"

// Load map file
void map_t::load(mem_t &m, const map_file_t &f) {
  if (f.magic != MAP_MAGIC) throw log_except("Invalid map magic!");

  // Load loading zones
  prevLoad = f.prevLoad;
  nextLoad = f.nextLoad;

  // Load cubes
  cubeCount = f.cubeCount;

  if (cubes) m.free(cubes);
  if (cubeCount) {
    cubes = (map_cube_t*)m.alloc(sizeof(map_cube_t)*cubeCount);

    for (uptr i = 0; i < cubeCount; ++i)
      cubes[i] = f.cubes[i];
  }

  // Load level atlas
  levelAtlas = f.levelAtlas;
}
