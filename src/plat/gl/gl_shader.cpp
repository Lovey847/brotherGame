#include "types.h"
#include "log.h"
#include "gl_shader.h"
#include "gl_glf.h"
#include "opengl.h"

static const char *shaderTypeStr(GLenum shaderType) {
	switch (shaderType) {
	case GL::VERTEX_SHADER:   return "Vertex shader error: ";
	case GL::FRAGMENT_SHADER: return "Fragment shader error: ";

	default: return "Unknown shader error: ";
	}
}

gl_shader_t::gl_shader_t(const char *code, GLenum type) {
	m_obj = GLF(GL::CreateShader(type));
	if (!m_obj) throw log_except("Cannot create shader object!");

	GLF(GL::ShaderSource(m_obj, 1, &code, NULL));
	GLF(GL::CompileShader(m_obj));

	GLint status = 0;
	GLF(GL::GetShaderiv(m_obj, GL::COMPILE_STATUS, &status));

	if (!status) {
		char errBuf[512];
		GLsizei errLen;

		GLF(GL::GetShaderInfoLog(m_obj, 512, &errLen, errBuf));
		GLF(GL::DeleteShader(m_obj));
		
		throw log_except("%s%s", shaderTypeStr(type), errBuf);
	}
}

gl_shader_program_t::gl_shader_program_t(const gl_shader_t &vertex, const gl_shader_t &fragment) {
	m_obj = GLF(GL::CreateProgram());
	if (!m_obj) throw log_except("Cannot create shader program object!");

	GLF(GL::AttachShader(m_obj, vertex.obj()));
	GLF(GL::AttachShader(m_obj, fragment.obj()));

	GLF(GL::LinkProgram(m_obj));

	GLF(GL::DetachShader(m_obj, vertex.obj()));
	GLF(GL::DetachShader(m_obj, fragment.obj()));

	GLint status;
	GLF(GL::GetProgramiv(m_obj, GL::LINK_STATUS, &status));

	if (!status) {
		char errBuf[512];
		GLsizei errLen;

		GLF(GL::GetProgramInfoLog(m_obj, 512, &errLen, errBuf));
		GLF(GL::DeleteProgram(m_obj));
		
		throw log_except("Program linking error: %s", errBuf);
	}

	/*
	const GLuint global = GLF(GL::GetUniformBlockIndex(m_obj, "global_t"));
	if (global == GL::INVALID_INDEX) {
		GL::DeleteProgram(m_obj);

		throw log_except("No uniform block named \"global_t\" in shader!");
	}
	
	GLF(GL::UniformBlockBinding(m_obj, global, GLBUFFER_UBO_GLOBAL));
	*/
}
