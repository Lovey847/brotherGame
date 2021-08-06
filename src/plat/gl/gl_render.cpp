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
	"  gl_Position = block.projection*block.modelView * inPos;\n"
	"  coord = inCoord;\n"
	"}\n";

static const char fragmentCode[] =
	"#version 330 core\n"
	"\n"
	"in vec2 coord;\n"
  "\n"
  "layout(std140) uniform block_t {\n"
  "  mat4 modelView;\n"
  "  mat4 projection;\n"
  "} block;\n"
	"\n"
  "uniform sampler2D tex;\n"
	"\n"
	"out vec4 fragCol;\n"
	"\n"
	"void main() {\n"
	"  fragCol = texture(tex, coord);\n"
	"}\n";

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
  m_projDist = 1.f/tanf(s.fovy*0.5f);

  // Near clipping plane distance
  static const f32 nearClip = 16.f;

  // Far clipping plane distance
  static const f32 farClip = 2048.f;

  m_buf.block().projection[0].f[0] = m_projDist*invAspect;
  m_buf.block().projection[1].f[1] = m_projDist;
  m_buf.block().projection[2] = vec4(0.f, 0.f, (farClip+nearClip*2.f)/farClip, 1.f);
  m_buf.block().projection[3] = vec4(0.f, 0.f, nearClip*-2.f, 0.f);

  // Enable depth buffer
  GLF(GL::Enable(GL::DEPTH_TEST));
  GLF(GL::DepthFunc(GL::LESS));
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

  // The origin of the yaw is pointing right
  const f32 c = cosf(state.game->yaw-(f32)M_PI*0.5f);
  const f32 s = sinf(state.game->yaw-(f32)M_PI*0.5f);

  const f32 pc = cosf(state.game->pitch);
  const f32 ps = sinf(state.game->pitch);

  const f32 x = state.game->pos.f[0];
  const f32 y = state.game->pos.f[1];
  const f32 z = state.game->pos.f[2];

  // Setup model view matrix, contains
  // transformation matrix, yaw rotation matrix and
  // pitch rotation matrix, performed on the vertex
  // in that order
  //
  // NOTE: This can be optimized by storing c, s,
  // pc and ps in a vector, and x, y, z in another vector,
  // and shuffling them around instead of referring to them
  // as singular values. I don't know anyone insane enough
  // to do that though.
  //
  // modelView =
  // c     0   s    c*-x+s*-z
  // ps*s  pc -ps*c ps*s*-x+pc*-y+ps*c*z
  // pc*-s ps  pc*c pc*s*x+ps*-y+pc*c*-z
  // 0     0   0    1
  m_buf.block().modelView[0] =
    vec4(c, ps, pc, 0.f)*vec4(1.f, s, -s, 1.f);
  m_buf.block().modelView[1] =
    vec4(0.f, pc, ps, 0.f);
  m_buf.block().modelView[2] =
    vec4(s, -ps, pc, 0.f)*vec4(1.f, c, c, 1.f);
  m_buf.block().modelView[3] =
    vec4(c, ps, pc, 1.f)*vec4(-x, s, s, 1.f)*vec4(1.f, -x, x, 1.f) +
    vec4(s, pc, ps, 0.f)*vec4(-z, -y, -y, 0.f) +
    vec4(0.f, ps, pc, 0.f)*vec4(0.f, c, c, 0.f)*vec4(0.f, z, -z, 0.f);

  // Add game cubes
  m_buf.addCube(m_texture, state.game->pos, state.game->cube);

  GLF(GL::Clear(GL::COLOR_BUFFER_BIT|GL::DEPTH_BUFFER_BIT));

  m_buf.flushBuffers();

	return true;
}

ubool gl_render_t::resize(u32 width, u32 height) {
  // Resize projection matrix
  m_buf.block().projection[0].f[0] = (f32)height/(f32)width*m_projDist;

	GLF(GL::Viewport(0, 0, width, height));

	return true;
}
