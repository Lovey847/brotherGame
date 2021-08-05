#ifndef GL_RENDER_H
#define GL_RENDER_H

#include "types.h"
#include "mem.h"
#include "game/state.h"

#include "opengl.h"
#include "gl_shader.h"
#include "gl_texture.h"
#include "gl_buffer.h"

class gl_render_t {
private:
	// Object to load functions before anything else constructs
	// TODO: I wanna do this in a different way, maybe load functions in the platform layer
	// after creating opengl context?
	class m_loadFuncs_t {
	public:
		FINLINE m_loadFuncs_t() {
			GL::loadExt();
		}
	} m_loadFuncs;
	
	mem_t &m_m;

	gl_shader_program_t m_program;

  gl_texture_t m_texture;

  gl_buffers_t m_buf;

public:
	gl_render_t(mem_t &m, const game_state_t &s, u32 width, u32 height);
	~gl_render_t();

	// Returns false if update/resize failed
	ubool render(game_state_render_t &state);
	ubool resize(u32 width, u32 height);
};

#endif //GL_RENDER_H
