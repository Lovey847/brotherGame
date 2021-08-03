#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#include "types.h"
#include "opengl.h"
#include "game/atlas.h"

// Pack multiple atlas's into single texture
static constexpr u32 GLTEXTURE_WIDTH = ATLAS_WIDTH*2;
static constexpr u32 GLTEXTURE_HEIGHT = ATLAS_HEIGHT;

// Atlas offset list
extern const ivec4 gl_texture_atlasOffset[ATLAS_COUNT];

class gl_texture_t {
private:
  GLuint m_tex; // Texture object handle

public:
  gl_texture_t();
  ~gl_texture_t();

  // Load atlas into texture
  // Overwrites previous atlas in spot
  // Returns false on failure
  ubool load(atlas_id_t id, const atlas_t *atlas);
};

#endif //GL_TEXTURE_H
