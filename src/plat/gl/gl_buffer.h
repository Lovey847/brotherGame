#ifndef GL_BUFFER_H
#define GL_BUFFER_H

#include "types.h"
#include "opengl.h"
#include "gl_vertex.h"
#include "game/map.h"
#include "game/atlas.h"

// Dither matrix size
static constexpr u32 GLBUFFER_DITHER_SIZE = 16;

struct gl_buffer_block_t {
  // NOTE: All matrices are column-major
  vec4 modelView[4];
  vec4 projection[4];
};

class gl_buffers_t {
private:
  mem_t &m_m;

  gl_vertex_t *m_verts;
  u16 *m_inds;

  uptr m_vertCount, m_indCount;
  uptr m_vertSize, m_indSize;
  uptr m_curVert, m_curInd;
  uptr m_baseVert, m_baseInd;

  GLuint m_vao, m_vbo, m_ebo, m_ubo;

  gl_buffer_block_t *m_block;

public:
  gl_buffers_t(mem_t &m, uptr vertCount, uptr indCount);
  ~gl_buffers_t();

  // Add vertices to buffer
  void addVerts(uptr vertCount, const gl_vertex_t *verts, uptr indCount, const u16 *inds);

  FINLINE void addTriangle(const gl_vertex_t verts[3]) {
    static const u16 triangleInds[3] = {0, 1, 2};
    addVerts(3, verts, 3, triangleInds);
  }

  FINLINE void addQuad(const gl_vertex_t verts[4]) {
    static const u16 quadInds[6] = {0, 1, 2, 3, 1, 2};
    addVerts(4, verts, 6, quadInds);
  }

  // Add persistent vertices to buffer
  void addBaseVerts(uptr vertCount, const gl_vertex_t *verts, uptr indCount, const u16 *inds);

  FINLINE void addBaseTriangle(const gl_vertex_t verts[3]) {
    static const u16 triangleInds[3] = {0, 1, 2};
    addBaseVerts(3, verts, 3, triangleInds);
  }

  FINLINE void addBaseQuad(const gl_vertex_t verts[4]) {
    static const u16 quadInds[6] = {0, 1, 2, 3, 1, 2};
    addBaseVerts(4, verts, 6, quadInds);
  }

  // Clear persistent vertices
  FINLINE void clearBaseVerts() {
    m_baseVert = m_baseInd = 0;
  }

  // Add game cube to screen
  void addCube(const gl_texture_t &tex, const map_cube_t &c, ubool base = true, atlas_id_t atlas = ATLAS_LEVEL);

  // Render buffer contents
  void flushBuffers();

  FINLINE const gl_buffer_block_t &block() const {return *m_block;}
  FINLINE gl_buffer_block_t &block() {return *m_block;}
};

#endif //GL_BUFFER_H
