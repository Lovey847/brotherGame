#include "types.h"
#include "opengl.h"
#include "gl_glf.h"
#include "game/atlas.h"
#include "gl_texture.h"

const ivec4 gl_texture_atlasOffset[ATLAS_COUNT] = {
  ivec4(0, 0, 0, 0), ivec4(ATLAS_WIDTH, 0, 0, 0)
};

gl_texture_t::gl_texture_t() {
  // Initialize texture object
  GLF(GL::GenTextures(1, &m_tex));
  GLF(GL::BindTexture(GL_TEXTURE_2D, m_tex));

  // Set texture parameters
  GLF(GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  GLF(GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  GLF(GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GLF(GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  GLF(GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));

  // Initialize texture image
  GLF(GL::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                     GLTEXTURE_WIDTH, GLTEXTURE_HEIGHT, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, NULL));
}

gl_texture_t::~gl_texture_t() {
  // Destroy texture image
  GLF(GL::DeleteTextures(1, &m_tex));
}

ubool gl_texture_t::load(atlas_id_t id, const atlas_t *atlas) {
  GLF(GL::TexSubImage2D(GL_TEXTURE_2D, 0,
                        gl_texture_atlasOffset[id].i[0], gl_texture_atlasOffset[id].i[1],
                        ATLAS_WIDTH, ATLAS_HEIGHT,
                        GL_RGBA, GL_UNSIGNED_BYTE, atlas->data));
  return true;
}
