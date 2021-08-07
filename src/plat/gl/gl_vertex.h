#ifndef GL_VERTEX_H
#define GL_VERTEX_H

#include "types.h"
#include "gl_texture.h"

struct gl_vertex_t {
  vec4 pos;
  vec2_2 coord; // Third component is color

  FINLINE u32 &col() {return *(u32*)(coord.i+2);}
  FINLINE const u32 &col() const {return *(const u32*)(coord.i+2);}
};

// Offset of color in vertex
static constexpr uptr GLVERTEX_COLOFFSET_UPTR = offsetof(gl_vertex_t, coord) + 2*sizeof(f32);
#define GLVERTEX_COLOFFSET ((void*)GLVERTEX_COLOFFSET_UPTR)

#endif //GL_VERTEX_H
