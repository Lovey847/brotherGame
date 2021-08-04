#include "types.h"
#include "opengl.h"
#include "gl_glf.h"
#include "game/atlas.h"
#include "gl_texture.h"

const ivec2_2 gl_texture_atlasOffset[ATLAS_COUNT] = {
  ivec2_2(0, 0, 0, 0), ivec2_2(ATLAS_WIDTH, 0, 0, 0)
};

gl_texture_t::gl_texture_t() :
  m_atlas{}
{
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

ivec2_2 gl_texture_t::imgCoord(atlas_id_t atlas, str_hash_t name) {
  // If there's no atlas in this spot, return zeros
  if (!m_atlas[atlas]) return ivec4(0);

  // Search atlas image names
  for (uptr i = m_atlas[atlas]->imageCount; i--;) {
    if (m_atlas[atlas]->imgNames[i] == name) {
      return gl_texture_atlasOffset[atlas]+m_atlas[atlas]->imgDim[i].v();
    }
  }

  return ivec4(0);
}

ubool gl_texture_t::load(atlas_id_t id, const atlas_t *atlas) {
  if (atlas) {
    GLF(GL::TexSubImage2D(GL_TEXTURE_2D, 0,
                          gl_texture_atlasOffset[id].i[0], gl_texture_atlasOffset[id].i[1],
                          ATLAS_WIDTH, ATLAS_HEIGHT,
                          GL_RGBA, GL_UNSIGNED_BYTE, atlas->data));
  }

  m_atlas[id] = atlas;

  return true;
}
