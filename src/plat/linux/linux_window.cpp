#include "types.h"
#include "window.h"
#include "linux_window.h"

// Backend includes
#ifdef PLAT_B_OPENGL
#include "linux_gl_window.h"
#endif

// Backend construction functions
#define GL_CONSTRUCT

#ifdef PLAT_B_OPENGL

static ubool glConstruct(window_base_t *out, window_init_t &args) {
	log_note("Constructing window module with opengl backend");
	
	try {
		(void)new(out) linux_gl_window_t(args);
		return true;
	} catch (const log_except_t &err) {
		log_warning("Cannot initialize window module with opengl backend: %s", err.str());
		return false;
	}
}

#undef GL_CONSTRUCT
#define GL_CONSTRUCT glConstruct,

#endif

// Backend constructor table
const module_constructProc_t<window_base_t, window_init_t> window_construct[WINDOW_COUNT] = {
	GL_CONSTRUCT
};
