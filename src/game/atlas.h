#ifndef GAME_ATLAS_H
#define GAME_ATLAS_H

#include "types.h"
#include "str.h"
#include "game/pak.h"
#include "util.h"

// List of image atlases
enum atlas_id_t {
  ATLAS_GLOBAL, // Global atlas, always loaded
  ATLAS_LEVEL, /* Level atlas, switches between levels
                * Doesn't have any images, since level
                * geometry uses texture coordinates that
                * lookup directly into the atlas
                */

  ATLAS_COUNT,
};

// Image atlas color format, RGBA8
union atlas_col_t {
  u32 p;
  u8 c[4]; // Individual colors
};

// Atlas image index
typedef uptr atlas_img_t;
static constexpr atlas_img_t ATLAS_INVALID_IMAGE = 0xffffffffu;

// Image atlas file format
static constexpr u32 ATLAS_MAGIC = util_magic('A', 'T', 'L', 'S');
struct atlas_t {
  u32 magic; // == ATLAS_MAGIC

  endian_u32 imageCount; // Number of images in atlas

  // Pointer to atlas image data
  pak_ptr_t<atlas_col_t> data;

  // Pointer to atlas image dimensions
  pak_ptr_t<endian_ivec4> imgDim;

  // Atlas image names
  str_hash_t imgNames[1 /*imageCount*/];

  // Get image from atlas, return ATLAS_INVALID_IMAGE if not found
  atlas_img_t getImg(str_hash_t name);
};

// Atlas dimensions
static constexpr u32 ATLAS_WIDTH = 1024;
static constexpr u32 ATLAS_HEIGHT = 1024;

#endif //GAME_ATLAS_H
