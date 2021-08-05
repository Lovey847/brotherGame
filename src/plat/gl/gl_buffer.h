#ifndef GL_BUFFER_H
#define GL_BUFFER_H

#include "types.h"
#include "opengl.h"
#include "gl_vertex.h"

struct gl_buffer_block_t {
  // NOTE: All matrices are column-major,
  // so access them like "projection[column][row]"
  f32 modelView[4][4];
  f32 projection[4][4];
};

class gl_buffers_t {
private:
  mem_t &m_m;

  gl_vertex_t *m_verts;
  u16 *m_inds;

  uptr m_vertCount, m_indCount;
  uptr m_vertSize, m_indSize;
  uptr m_curVert, m_curInd;

  GLuint m_vao, m_vbo, m_ebo, m_ubo;

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

  // Render buffer contents
  void flushBuffers();
};

#endif //GL_BUFFER_H
