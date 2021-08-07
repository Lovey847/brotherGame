#include "types.h"
#include "opengl.h"
#include "gl_glf.h"
#include "game/atlas.h"
#include "gl_texture.h"

const vec2_2 gl_texture_atlasOffset[ATLAS_COUNT] = {
  vec2_2(0.f, 0.f, 0.f, 0.f),
  vec2_2((f32)ATLAS_WIDTH/(f32)GLTEXTURE_WIDTH, 0.f, 0.f, 0.f),
};

gl_texture_t::gl_texture_t() :
  m_atlas{}
{
  // Initialize texture object
  GLF(GL::GenTextures(1, &m_tex));
  GLF(GL::BindTexture(GL::TEXTURE_2D, m_tex));

  // Set texture parameters
  GLF(GL::TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_WRAP_S, GL::CLAMP_TO_EDGE));
  GLF(GL::TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_WRAP_T, GL::CLAMP_TO_EDGE));
  GLF(GL::TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MIN_FILTER, GL::LINEAR));
  GLF(GL::TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MAG_FILTER, GL::LINEAR));
  GLF(GL::TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MAX_LEVEL, 0));

  // Initialize texture image
  GLF(GL::TexImage2D(GL::TEXTURE_2D, 0, GL::RGBA8,
                     GLTEXTURE_WIDTH, GLTEXTURE_HEIGHT, 0,
                     GL::RGBA, GL::UNSIGNED_BYTE, NULL));
}

gl_texture_t::~gl_texture_t() {
  // Destroy texture image
  GLF(GL::DeleteTextures(1, &m_tex));
}

vec2_2 gl_texture_t::imgCoord(atlas_id_t atlas, str_hash_t name) const {
  // Atlas coordinate normalizer
  static const vec2_2 normMul(1.f/(f32)GLTEXTURE_WIDTH, 1.f/(f32)GLTEXTURE_HEIGHT,
                              1.f/(f32)GLTEXTURE_WIDTH, 1.f/(f32)GLTEXTURE_HEIGHT);

  // If there's no atlas in this spot, return zeros
  if (!m_atlas[atlas]) return vec2_2(0.f);

  // Search atlas image names
  for (uptr i = m_atlas[atlas]->imageCount; i--;) {
    // atlas imgDim, normalized
    if (m_atlas[atlas]->imgNames[i] == name) {
      return gl_texture_atlasOffset[atlas]+vec4_ivec4(m_atlas[atlas]->imgDim[i].v())*normMul;
    }
  }

  return vec2_2(0.f);
}

ubool gl_texture_t::load(atlas_id_t id, const atlas_t *atlas) {
  static const u32 atlasIntOffset[ATLAS_COUNT][2] = {
    {0, 0},
    {ATLAS_WIDTH, 0},
  };

  if (atlas) {
    GLF(GL::TexSubImage2D(GL::TEXTURE_2D, 0,
                          atlasIntOffset[id][0], atlasIntOffset[id][1],
                          ATLAS_WIDTH, ATLAS_HEIGHT,
                          GL::RGBA, GL::UNSIGNED_BYTE, atlas->data));
  }

  m_atlas[id] = atlas;

  return true;
}
