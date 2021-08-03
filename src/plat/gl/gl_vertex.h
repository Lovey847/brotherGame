#ifndef GL_VERTEX_H
#define GL_VERTEX_H

#include "types.h"
#include "gl_texture.h"

struct gl_vertex_t {
  
};

// Make sure we're correct in our size assessment here
static_assert(sizeof(gl_vertex_t) == 64, "");

#endif //GL_VERTEX_H
