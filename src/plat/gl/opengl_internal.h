// What this file does:
//
// This includes files and makes defines for opengl.h based on the platform
// The following symbols are defined:
//
//  OPENGL_GL_INCLUDED: Defined if gl.h is included

#ifndef OPENGL_INTERNAL_H

#if defined(PLAT_OS_WIN)

#include <windows.h>
#include <GL/gl.h>

#define OPENGL_GL_INCLUDED 1

#elif defined(PLAT_OS_LINUX)

#include <GL/gl.h>

#define OPENGL_GL_INCLUDED 1

#else

#error OpenGL not supported on this platform!

#endif //Platform detection

#endif //OPENGL_INTERNAL_H
