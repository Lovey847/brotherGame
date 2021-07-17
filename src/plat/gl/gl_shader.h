#ifndef GL_SHADER_H
#define GL_SHADER_H

#include "types.h"
#include "opengl.h"
#include "gl_glf.h"
#include "log.h"

class gl_shader_t {
private:
	GLuint m_obj;

public:
	gl_shader_t(const char *code, GLenum type);

	gl_shader_t(const gl_shader_t&) = delete;

	FINLINE ~gl_shader_t() {
		GLF(GL::DeleteShader(m_obj));
	}

	FINLINE GLuint obj() const {return m_obj;}
};

class gl_shader_program_t {
private:
	GLuint m_obj;

public:
	gl_shader_program_t(const gl_shader_t &vertex, const gl_shader_t &fragment);
	FINLINE gl_shader_program_t(const char *vertexCode, const char *fragmentCode) :
		gl_shader_program_t(gl_shader_t(vertexCode, GL::VERTEX_SHADER),
							gl_shader_t(fragmentCode, GL::FRAGMENT_SHADER)) {}

	gl_shader_program_t(const gl_shader_program_t&) = delete;

	FINLINE ~gl_shader_program_t() {
		GLF(GL::DeleteProgram(m_obj));
	}

	FINLINE void use() {
		GLF(GL::UseProgram(m_obj));
	}

	FINLINE GLuint obj() const {return m_obj;}
};

#endif //GL_SHADER_H
