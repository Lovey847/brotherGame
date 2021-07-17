#include "opengl.h"
#include "log.h"

#include <GL/glx.h>
#include <GL/glxext.h>

#define DEFEXT(_type, _name, ...)				\
	GL::_name ## _t GL::_name = NULL;

OPENGL_EXTFUNC_LIST

#undef DEFEXT

ubool GL::loadExt() {
#define DEFEXT(_type, _name, ...)				\
	GL::_name = (GL::_name ## _t)glXGetProcAddress((const GLubyte*)"gl" #_name);\
	if (!GL::_name) {\
		log_warning("Cannot load gl" #_name);\
		return false;\
	}

	OPENGL_EXTFUNC_LIST

#undef DEFEXT

	return true;
}
