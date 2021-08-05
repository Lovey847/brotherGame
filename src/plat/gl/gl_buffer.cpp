#include "types.h"
#include "gl_buffer.h"
#include "gl_texture.h"
#include "gl_glf.h"
#include "opengl.h"
#include "game/cube.h"

gl_buffers_t::gl_buffers_t(mem_t &m, uptr vertCount, uptr indCount) : m_m(m) {
  // Buffer sizes, used when orphaning buffers
  m_vertSize = vertCount*sizeof(gl_vertex_t);
  m_indSize = indCount*sizeof(u16);

  // Create VAO
  GLF(GL::GenVertexArrays(1, &m_vao));
  GLF(GL::BindVertexArray(m_vao));

  // Create VBO
  GLF(GL::GenBuffers(1, &m_vbo));
  GLF(GL::BindBuffer(GL::ARRAY_BUFFER, m_vbo));

  // Create EBO
  GLF(GL::GenBuffers(1, &m_ebo));
  GLF(GL::BindBuffer(GL::ELEMENT_ARRAY_BUFFER, m_ebo));

  // Create UBO
  GLF(GL::GenBuffers(1, &m_ubo));
  GLF(GL::BindBuffer(GL::UNIFORM_BUFFER, m_ubo));

  // Setup VAO attributes
  GLF(GL::VertexAttribPointer(0,
                              4, GL::FLOAT, GL::FALSE,
                              sizeof(gl_vertex_t), (void*)0));
  GLF(GL::VertexAttribPointer(1,
                              2, GL::FLOAT, GL::FALSE,
                              sizeof(gl_vertex_t), (void*)offsetof(gl_vertex_t, coord)));
  GLF(GL::EnableVertexAttribArray(0));
  GLF(GL::EnableVertexAttribArray(1));

  // Bind uniform buffer range
  GLF(GL::BindBufferRange(GL::UNIFORM_BUFFER, 0, m_ubo, 0, sizeof(gl_buffer_block_t)));

  // Allocate vertices, indices and the uniform block in one buffer
  const u8 *memory = (u8*)m_m.alloc(sizeof(gl_buffer_block_t)+m_vertSize+m_indSize);
  m_block = (gl_buffer_block_t*)memory;
  m_verts = (gl_vertex_t*)(memory+sizeof(gl_buffer_block_t));
  m_inds = (u16*)(memory+sizeof(gl_buffer_block_t)+m_vertSize);

  m_curVert = m_curInd = 0;
}

gl_buffers_t::~gl_buffers_t() {
  m_m.free(m_block);

  GLF(GL::DeleteBuffers(1, &m_ubo));
  GLF(GL::DeleteBuffers(1, &m_ebo));
  GLF(GL::DeleteBuffers(1, &m_vbo));
  GLF(GL::DeleteVertexArrays(1, &m_vao));
}

void gl_buffers_t::addVerts(uptr vertCount, const gl_vertex_t *verts,
                            uptr indCount, const u16 *inds)
{
  if (!(vertCount+indCount)) return;

  if (m_curVert+vertCount > m_vertCount)
    throw log_except("Out of vertex memory! (%u > %u)",
                     (unsigned)(m_curVert+vertCount),
                     (unsigned)m_vertCount);

  if (m_curInd+indCount > m_indCount)
    throw log_except("Out of index memory! (%u > %u)",
                     (unsigned)(m_curInd+indCount),
                     (unsigned)m_indCount);

  memcpy((void*)(m_verts+m_curVert), verts, vertCount*sizeof(gl_vertex_t));

  // We have to adjust the indices
  for (uptr i = 0; i < indCount; ++i)
    m_inds[m_curInd++] = inds[i]+m_curVert;

  m_curVert += vertCount;
}

void gl_buffers_t::addCube(const gl_texture_t &tex, vec4 pos, const cube_t &c) {
  gl_vertex_t verts[4]; // Quad vertices, drawn 4 times

  // Setup texture coordinates
  // TODO: This should load from ATLAS_LEVEL!
  verts[0].coord = tex.imgCoord(ATLAS_GLOBAL, c.img);
  verts[1].coord = (verts[0].coord +
                    (verts[0].coord.shuffle<0x2323>()&vec4(vec4_int_init(-1, 0, 0, 0))));
  verts[2].coord = (verts[0].coord +
                    (verts[0].coord.shuffle<0x2323>()&vec4(vec4_int_init(0, -1, 0, 0))));
  verts[3].coord = (verts[0].coord +
                    (verts[0].coord.shuffle<0x2323>()&vec4(vec4_int_init(-1, -1, 0, 0))));

  // Left side
  if (pos.f[0] < c.min.f[0]) {
    verts[0].pos = ((c.min&vec4(vec4_int_init(-1, 0, 0, -1))) |
                    (c.max&vec4(vec4_int_init(0, -1, -1, 0))));
    verts[1].pos = ((c.min&vec4(vec4_int_init(-1, 0, -1, -1))) |
                    (c.max&vec4(vec4_int_init(0, -1, 0, 0))));
    verts[2].pos = ((c.min&vec4(vec4_int_init(-1, -1, 0, -1))) |
                    (c.max&vec4(vec4_int_init(0, 0, -1, 0))));
    verts[3].pos = c.min;

    addQuad(verts);
  }

  // Right side
  else if (pos.f[0] > c.max.f[0]) {
    verts[0].pos = ((c.min&vec4(vec4_int_init(0, 0, -1, -1))) |
                    (c.max&vec4(vec4_int_init(-1, -1, 0, 0))));
    verts[1].pos = c.max;
    verts[2].pos = ((c.min&vec4(vec4_int_init(0, -1, -1, -1))) |
                    (c.max&vec4(vec4_int_init(-1, 0, 0, 0))));
    verts[3].pos = ((c.min&vec4(vec4_int_init(0, -1, 0, -1))) |
                    (c.max&vec4(vec4_int_init(-1, 0, -1, 0))));

    addQuad(verts);
  }

  // Bottom side
  if (pos.f[1] < c.min.f[1]) {
    verts[0].pos = c.min;
    verts[1].pos = ((c.min&vec4(vec4_int_init(0, -1, -1, -1))) |
                    (c.max&vec4(vec4_int_init(-1, 0, 0, 0))));
    verts[2].pos = ((c.min&vec4(vec4_int_init(-1, -1, 0, -1))) |
                    (c.max&vec4(vec4_int_init(0, 0, -1, 0))));
    verts[3].pos = ((c.min&vec4(vec4_int_init(0, -1, 0, -1))) |
                    (c.max&vec4(vec4_int_init(-1, 0, -1, 0))));

    addQuad(verts);
  }

  // Top side
  else if (pos.f[1] > c.max.f[1]) {
    verts[0].pos = ((c.min&vec4(vec4_int_init(-1, 0, 0, -1))) |
                    (c.max&vec4(vec4_int_init(0, -1, -1, 0))));
    verts[1].pos = c.max;
    verts[2].pos = ((c.min&vec4(vec4_int_init(-1, 0, -1, -1))) |
                    (c.max&vec4(vec4_int_init(0, -1, 0, 0))));
    verts[3].pos = ((c.min&vec4(vec4_int_init(0, 0, -1, -1))) |
                    (c.max&vec4(vec4_int_init(-1, -1, 0, 0))));

    addQuad(verts);
  }

  // Front side
  if (pos.f[2] < c.min.f[2]) {
    verts[0].pos = ((c.min&vec4(vec4_int_init(-1, 0, -1, -1))) |
                    (c.max&vec4(vec4_int_init(0, -1, 0, 0))));
    verts[1].pos = ((c.min&vec4(vec4_int_init(0, 0, -1, -1))) |
                    (c.max&vec4(vec4_int_init(-1, -1, 0, 0))));
    verts[2].pos = c.min;
    verts[3].pos = ((c.min&vec4(vec4_int_init(0, -1, -1, -1))) |
                    (c.max&vec4(vec4_int_init(-1, 0, 0, 0))));

    addQuad(verts);
  }

  // Back side
  else if (pos.f[2] > c.max.f[2]) {
    verts[0].pos = c.max;
    verts[1].pos = ((c.min&vec4(vec4_int_init(-1, 0, 0, -1))) |
                    (c.max&vec4(vec4_int_init(0, -1, -1, 0))));
    verts[2].pos = ((c.min&vec4(vec4_int_init(0, -1, 0, -1))) |
                    (c.max&vec4(vec4_int_init(-1, 0, -1, 0))));
    verts[3].pos = ((c.min&vec4(vec4_int_init(-1, -1, 0, -1))) |
                    (c.max&vec4(vec4_int_init(0, 0, -1, 0))));

    addQuad(verts);
  }
}

void gl_buffers_t::flushBuffers() {
  // Orphan buffers
  GLF(GL::BufferData(GL::ARRAY_BUFFER, m_vertSize, NULL, GL::STREAM_DRAW));
  GLF(GL::BufferSubData(GL::ARRAY_BUFFER, 0, m_curVert*sizeof(gl_vertex_t), m_verts));
  GLF(GL::BufferData(GL::ELEMENT_ARRAY_BUFFER, m_indSize, NULL, GL::STREAM_DRAW));
  GLF(GL::BufferSubData(GL::ELEMENT_ARRAY_BUFFER, 0, m_curInd*sizeof(u16), m_inds));
  GLF(GL::BufferData(GL::UNIFORM_BUFFER, sizeof(gl_buffer_block_t), NULL, GL::STREAM_DRAW));
  GLF(GL::BufferSubData(GL::UNIFORM_BUFFER, 0, sizeof(gl_buffer_block_t), m_block));

  // Render vertices
  GLF(GL::DrawElements(GL::TRIANGLES, m_curInd, GL::UNSIGNED_SHORT, (void*)0));

  // Reset buffers
  m_curVert = m_curInd = 0;
}
