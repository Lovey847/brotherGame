#include "types.h"
#include "endianUtil.h"

#include "gl_render.h"
#include "gl_shader.h"
#include "gl_glf.h"

// NOTE: Think of a better system for shaders at some point!
static const char vertexCode[] =
	"#version 330 core\n"
	"\n"
	"layout(location = 0) in vec2 inPos;\n"
	"layout(location = 1) in vec4 inCol;\n"
	"\n"
	"uniform vec2 adjust;\n"
	"\n"
	"out vec4 col;\n"
	"\n"
	"void main() {\n"
	"    gl_Position = vec4(inPos+adjust, 0.0, 1.0);\n"
	"    col = inCol;\n"
	"}\n";

static const char fragmentCode[] =
	"#version 330 core\n"
	"\n"
	"in vec4 col;\n"
	"\n"
	"out vec4 fragCol;\n"
	"\n"
	"void main() {\n"
	"    fragCol = col;\n"
	"}\n";

struct vert {
	f32 x, y;
	u32 col;
};

static const vert verts[] = {
	{-0.5f, 0.f, endian_little32(0xff00ff00)},
	{0.5f, 0.5f, endian_little32(0xffff0000)},
	{0.5f, -0.5f, endian_little32(0xff0000ff)}
};

gl_render_t::gl_render_t(mem_t &m, u32 width, u32 height) :
	m_m(m), m_program(vertexCode, fragmentCode)
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
	
	// Get "adjust" uniform
	m_adjust = GLF(GL::GetUniformLocation(m_program.obj(), "adjust"));
	
	// Set clear color
	GLF(GL::ClearColor(0.f, 0.f, 0.f, 0.f));

	// Set viewport
	resize(width, height);

	// Make VAO
	GLF(GL::GenVertexArrays(1, &m_vao));
	GLF(GL::BindVertexArray(m_vao));

	// Make VBO
	GLF(GL::GenBuffers(1, &m_vbo));
	GLF(GL::BindBuffer(GL::ARRAY_BUFFER, m_vbo));
	GLF(GL::BufferData(GL::ARRAY_BUFFER, sizeof(verts), verts, GL::STATIC_DRAW));

	// Set VAO settings
	GLF(GL::VertexAttribPointer(0, 2, GL::FLOAT, GL_FALSE, sizeof(vert), (void*)0));
	GLF(GL::VertexAttribPointer(1, GL::BGRA, GL::UNSIGNED_BYTE, GL_TRUE, sizeof(vert), (void*)(sizeof(GLfloat)*2)));
	GLF(GL::EnableVertexAttribArray(0));
	GLF(GL::EnableVertexAttribArray(1));

	// Use shader
	m_program.use();
}

gl_render_t::~gl_render_t() {
	// Destroy VBO
	GLF(GL::BindBuffer(GL::ARRAY_BUFFER, 0));
	GLF(GL::DeleteBuffers(1, &m_vbo));

	// Destroy VAO
	GLF(GL::BindVertexArray(0));
	GLF(GL::DeleteVertexArrays(1, &m_vao));
}

ubool gl_render_t::render(const game_state_t &state) {
	GLF(GL::Uniform2f(m_adjust, state.x*0.5f, state.y*0.5f));
	
	GLF(GL::Clear(GL::COLOR_BUFFER_BIT));
	GLF(GL::DrawArrays(GL::TRIANGLES, 0, 3));

	return true;
}

ubool gl_render_t::resize(u32 width, u32 height) {
	GLF(GL::Viewport(0, 0, width, height));

	return true;
}

void interpEntities(ubool interp) {
	(void)interp;
}
