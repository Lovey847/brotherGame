#include "types.h"
#include "endianUtil.h"

#include "gl_render.h"
#include "gl_shader.h"
#include "gl_buffer.h"
#include "gl_glf.h"

#include <math.h>

// NOTE: Think of a better system for shaders at some point!
static const char vertexCode[] =
	"#version 330 core\n"
	"\n"
	"layout(location = 0) in vec4 inPos;\n"
	"layout(location = 1) in vec2 inCoord;\n"
  "\n"
  "layout(std140) uniform block_t {\n"
  "  mat4 modelView;\n"
  "  mat4 projection;\n"
  "} block;\n"
	"\n"
	"out vec2 coord;\n"
	"\n"
	"void main() {\n"
	"    gl_Position = block.projection*block.modelView * inPos;\n"
	"    coord = inCoord;\n"
	"}\n";

static const char fragmentCode[] =
	"#version 330 core\n"
	"\n"
	"in vec2 coord;\n"
  "\n"
  "uniform sampler2D tex;\n"
	"\n"
	"out vec4 fragCol;\n"
	"\n"
	"void main() {\n"
	"    fragCol = texture(tex, coord);\n"
	"}\n";

static const gl_vertex_t verts[] = {
  {vec4(-0.5f, 0.f, 0.f, 1.f), vec2_2(0.f, 128.f/(float)GLTEXTURE_HEIGHT, 0.f, 0.f)},
  {vec4(0.5f, 0.5f, 0.f, 1.f), vec2_2(256.f/(float)GLTEXTURE_WIDTH, 0.f, 0.f, 0.f)},
  {vec4(0.5f, -0.5f, 0.f, 1.f), vec2_2(256.f/(float)GLTEXTURE_WIDTH, 256.f/(float)GLTEXTURE_HEIGHT, 0.f, 0.f)}
};

static const vec4 identMat[4] = {
  vec4(1.f, 0.f, 0.f, 0.f),
  vec4(0.f, 1.f, 0.f, 0.f),
  vec4(0.f, 0.f, 1.f, 0.f),
  vec4(0.f, 0.f, 0.f, 1.f)
};

gl_render_t::gl_render_t(mem_t &m, const game_state_t &s, u32 width, u32 height) :
	m_m(m), m_program(vertexCode, fragmentCode),
  m_buf(m, 2048, 3072)
{
	// Log vendor info
	const char * const vendor = (const char*)GLF(GL::GetString(GL::VENDOR));
	const char * const renderer = (const char*)GLF(GL::GetString(GL::RENDERER));
	const char * const version = (const char*)GLF(GL::GetString(GL::VERSION));
	const char * const glslVersion = (const char*)GLF(GL::GetString(GL::SHADING_LANGUAGE_VERSION));

	log_note("\n"
			 "OpenGL vendor:   %s\n"
			 "OpenGL renderer: %s\n"
			 "OpenGL version:  %s\n"
			 "GLSL version:    %s",

			 vendor, renderer, version, glslVersion);

	// Set clear color
	GLF(GL::ClearColor(0.f, 0.f, 0.f, 0.f));

	// Set viewport
	resize(width, height);

	// Use shader
	m_program.use();

  // Setup projection matrix
  // Model view matrix is setup when rendering
  memcpy((void*)m_buf.block().projection, &identMat, sizeof(identMat));

  const f32 invAspect = (f32)height/(f32)width;

  // Distance to projection plane
  const f32 projDist = 1.f/tanf(s.fovy*0.5f);

  // Near clipping plane distance
  static const f32 nearClip = 32.f;

  // Far clipping plane distance
  static const f32 farClip = 2048.f;

  m_buf.block().projection[0].f[0] = projDist*invAspect;
  m_buf.block().projection[1].f[1] = projDist;
  m_buf.block().projection[2] = vec4(0.f, 0.f, (farClip+nearClip*2.f)/farClip, 1.f);
  m_buf.block().projection[3] = vec4(0.f, 0.f, nearClip*-2.f, 0.f);
}

gl_render_t::~gl_render_t() {
}

ubool gl_render_t::render(game_state_render_t &state) {
  // Check if we should load any atlases
  for (atlas_id_t i = 0; i < ATLAS_COUNT; ++i) {
    if (state.atlas[i]) {
      m_texture.load(i, state.atlas[i]);
      state.atlas[i] = NULL;
    }
  }

  // Setup model view matrix
  memcpy((void*)m_buf.block().modelView, &identMat, sizeof(identMat));

  m_buf.block().modelView[3] = -state.game->pos;
  m_buf.block().modelView[3].f[3] = 1.f;

  // Add block to buffer
  gl_vertex_t v[4];

  v[0].pos = state.game->cube.min;
  v[0].coord = m_texture.imgCoord(ATLAS_GLOBAL, str_hash("xor"));

  v[1].pos = vec4(state.game->cube.max.f[0], state.game->cube.min.f[1], state.game->cube.min.f[2], 1.f);
  v[1].coord = v[0].coord + (v[0].coord.shuffle<0x2123>()&vec4(vec4_int_init(0xffffffffu, 0, 0, 0)));

  v[2].pos = vec4(state.game->cube.min.f[0], state.game->cube.max.f[1], state.game->cube.min.f[2], 1.f);
  v[2].coord = v[0].coord + (v[0].coord.shuffle<0x0323>()&vec4(vec4_int_init(0, 0xffffffffu, 0, 0)));

  v[3].pos = vec4(state.game->cube.max.f[0], state.game->cube.max.f[1], state.game->cube.min.f[2], 1.f);
  v[3].coord = v[0].coord + v[0].coord.shuffle<0x2323>();

  m_buf.addQuad(v);

  v[0].pos = state.game->cube.min;
  v[1].pos = vec4(state.game->cube.min.f[0], state.game->cube.min.f[1], state.game->cube.max.f[2], 1.f);
  v[2].pos = vec4(state.game->cube.min.f[0], state.game->cube.max.f[1], state.game->cube.min.f[2], 1.f);
  v[3].pos = vec4(state.game->cube.min.f[0], state.game->cube.max.f[1], state.game->cube.max.f[2], 1.f);

  m_buf.addQuad(v);

  v[0].pos = vec4(state.game->cube.max.f[0], state.game->cube.min.f[1], state.game->cube.min.f[2], 1.f);;
  v[1].pos = vec4(state.game->cube.max.f[0], state.game->cube.min.f[1], state.game->cube.max.f[2], 1.f);
  v[2].pos = vec4(state.game->cube.max.f[0], state.game->cube.max.f[1], state.game->cube.min.f[2], 1.f);
  v[3].pos = vec4(state.game->cube.max.f[0], state.game->cube.max.f[1], state.game->cube.max.f[2], 1.f);

  m_buf.addQuad(v);

  GLF(GL::Clear(GL_COLOR_BUFFER_BIT));

  m_buf.flushBuffers();

	return true;
}

ubool gl_render_t::resize(u32 width, u32 height) {
	GLF(GL::Viewport(0, 0, width, height));

	return true;
}
