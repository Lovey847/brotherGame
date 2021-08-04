#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#include "types.h"
#include "opengl.h"
#include "game/atlas.h"

// Pack multiple atlas's into single texture
static constexpr u32 GLTEXTURE_WIDTH = ATLAS_WIDTH*2;
static constexpr u32 GLTEXTURE_HEIGHT = ATLAS_HEIGHT;

// Atlas offset list
extern const ivec2_2 gl_texture_atlasOffset[ATLAS_COUNT];

class gl_texture_t {
private:
  GLuint m_tex; // Texture object handle

  // Atlas list
  const atlas_t *m_atlas[ATLAS_COUNT];

public:
  gl_texture_t();
  ~gl_texture_t();

  // Get image offset in texture
  // z&w contain width and height
  ivec2_2 imgCoord(atlas_id_t atlas, str_hash_t name);

  // Load atlas into texture
  // Overwrites previous atlas in spot
  // If atlas is NULL, frees atlas in spot
  // Returns false on failure
  ubool load(atlas_id_t id, const atlas_t *atlas);
};

#endif //GL_TEXTURE_H
