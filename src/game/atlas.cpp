#include "types.h"
#include "atlas.h"

atlas_img_t atlas_t::getImg(str_hash_t name) {
  // Search through imgNames
  for (str_hash_t *n = imgNames + imageCount; n-- != imgNames;) {
    if (*n == name) return n-imgNames;
  }

  return ATLAS_INVALID_IMAGE;
}
