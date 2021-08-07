#include "gen.h"
#include "str.h"
#include "util.h"
#include "endianUtil.h"
#include "file.h"
#include "log.h"
#include "game/map.h"

#include <cstring>

void gen_main(mem_t &m, file_system_t &f, const char *txt, const char *output) {
  (void)m; (void)output; (void)txt;
  file_handle_t *first = f.open("../data/files/maps/first.map", FILE_MODE_READWRITE);
  if (!first) throw log_except("Cannot open ../data/files/maps/first.map!");

  const u8 zeros[sizeof(map_file_t)] = {};
  first->write(zeros, sizeof(map_file_t));

  file_mapping_t *fileMap = first->map(FILE_MAP_READWRITE, 0, sizeof(map_file_t));
  if (!fileMap) {
    first->close();
    throw log_except("Cannot map ../data/files/maps/first.map!");
  }

  map_file_t *map = (map_file_t*)fileMap->data;

  map->magic = MAP_MAGIC;
  map->cubeCount = 0;
  map->prevLoad.min = map->prevLoad.max = map->nextLoad.min = map->nextLoad.max = vec4(0.f);
  map->prevLoad.map = map->nextLoad.map = str_hash("maps/first.map");
  map->levelAtlas = str_hash("atlases/global.atl");
  map->cubes[0].min = vec4(-128.f, -384.f, -128.f, 1.f);
  map->cubes[0].max = vec4(128.f, -128.f, 128.f, 1.f);
  map->cubes[0].img = str_hash("tutorial");

  fileMap->unmap();

  first->close();
}
