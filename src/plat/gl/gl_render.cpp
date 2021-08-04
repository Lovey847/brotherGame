#include "types.h"
#include "endianUtil.h"

#include "gl_render.h"
#include "gl_shader.h"
#include "gl_buffer.h"
#include "gl_glf.h"

// NOTE: Think of a better system for shaders at some point!
static const char vertexCode[] =
	"#version 330 core\n"
	"\n"
	"layout(location = 0) in vec4 inPos;\n"
	"layout(location = 1) in vec2 inCoord;\n"
	"\n"
	"out vec2 coord;\n"
	"\n"
	"void main() {\n"
	"    gl_Position = inPos;\n"
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

gl_render_t::gl_render_t(mem_t &m, u32 width, u32 height) :
	m_m(m), m_program(vertexCode, fragmentCode), m_buf(m, 2048, 3072)
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
}

gl_render_t::~gl_render_t() {
}

ubool gl_render_t::render(const game_state_t &state) {
  // Check if we should load any atlases
  for (atlas_id_t i = 0; i < ATLAS_COUNT; ++i) {
    if (state.r.atlas[i] && (state.r.atlas[i] != m_texture.atlas(i))) {
      m_texture.load(i, state.r.atlas[i]);
    }
  }

  gl_vertex_t v[3];
  memcpy((void*)v, verts, sizeof(verts));

  for (uptr i = 0; i < sizeof(v)/sizeof(v[0]); ++i) {
    v[i].pos.f[0] += state.x;
    v[i].pos.f[1] += state.y;
  }

  GLF(GL::Clear(GL_COLOR_BUFFER_BIT));

  // Add verts triangle
  m_buf.addTriangle(v);

  m_buf.flushBuffers();

	return true;
}

ubool gl_render_t::resize(u32 width, u32 height) {
	GLF(GL::Viewport(0, 0, width, height));

	return true;
}

void interpEntities(ubool interp) {
	(void)interp;
}
