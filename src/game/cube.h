#ifndef CUBE_H
#define CUBE_H

#include "types.h"
#include "str.h"

// Used as bounding boxes, also drawn to the screen
struct cube_t {
  vec4 min, max;

  str_hash_t img; // Image from level atlas
};

#endif //CUBE_H
