#ifndef GL_GLF_H
#define GL_GLF_H

#include "types.h"
#include "opengl.h"
#include "log.h"

#ifdef NDEBUG

#define GLF(...) (__VA_ARGS__)

#else

static const char *GLF_ErrorString(GLenum errorCode) {
	switch (errorCode) {
	case GL::INVALID_ENUM:					return "INVALID_ENUM";
	case GL::INVALID_VALUE:					return "INVALID_VALUE";
	case GL::INVALID_OPERATION:				return "INVALID_OPERATION";
	case GL::INVALID_FRAMEBUFFER_OPERATION:	return "INVALID_FRAMEBUFFER_OPERATION";
	case GL::OUT_OF_MEMORY:					return "OUT_OF_MEMORY";
	case GL::STACK_UNDERFLOW:				return "STACK_UNDERFLOW";
	case GL::STACK_OVERFLOW:				return "STACK_OVERFLOW";
	}
	
	return "UNKNOWN_ERROR";
}

// Extreme debug version
#if 0

#define GLF(...) (__VA_ARGS__); {										\
		int ret;														\
		if ((ret = GL::GetError()) != GL::NO_ERROR) {					\
			log_warning("Opengl error: %s (" #__VA_ARGS__ ")", GLF_ErrorString(ret)); \
		} else {														\
			log_note("Opengl function call: " #__VA_ARGS__);			\
		}																\
	}

// Usual debug version
#else

#define GLF(...) (__VA_ARGS__); {										\
	    int ret;														\
		if ((ret = GL::GetError()) != GL::NO_ERROR) {					\
			log_warning("Opengl error: %s (" #__VA_ARGS__ ")", GLF_ErrorString(ret)); \
		}																\
	}

#endif

#endif // NDEBUG

#endif //GL_GLF_H
