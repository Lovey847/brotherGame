#ifndef LINUX_GL_WINDOW_H
#define LINUX_GL_WINDOW_H

#include "types.h"
#include "game/game.h"
#include "gl_render.h"
#include "modules.h"
#include "interfaces.h"
#include "window.h"

// Xlib
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>

// XShm
#if 0 // I don't think I use XShm

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

#endif

// OpenGL
#include <GL/glx.h>
#include <GL/glxext.h>

class linux_gl_window_t : public window_base_t {
private:
	modules_t &m_m;
	interfaces_t &m_i;

	// Container struct to initialize OpenGL context before gl_render_t
	// Also destroys OpenGL context after gl_render_t
	struct x_t {
		Display *dis;
		Window win;
		Atom WM_DELETE_WINDOW;

		int scr, root;

		Colormap cmap;

		GLXContext ctx;

		x_t();
		~x_t();
	} x;

	gl_render_t m_gl;

	// Keyboard keycode map
	KeySym *m_map;
	int m_minCode, m_maxCode;
	int m_symPerCode;

public:
	linux_gl_window_t(window_init_t &init);
	~linux_gl_window_t();

	window_loop_ret_t loop(game_t &game);
};

#endif //LINUX_GL_WINDOW_H
