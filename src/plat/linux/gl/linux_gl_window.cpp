#include "types.h"
#include "linux_window.h"
#include "linux_gl_window.h"
#include "linux_key.h"

linux_gl_window_t::x_t::x_t() {
	// Open display
	dis = XOpenDisplay(NULL);
	if (!dis) throw log_except("Couldn't open display!");

	// Get default screen and root window of display
	scr = DefaultScreen(dis);
	root = DefaultRootWindow(dis);

	// Get GLX framebuffer configurations
	static const int GLXConfigAttribs[] = {
		GLX_DOUBLEBUFFER, True, // Double-buffered configuration
		
		GLX_RED_SIZE, 8, // At least 8-bits in red, green, blue and alpha
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		
		GLX_DEPTH_SIZE, 24, // At least 24-bits in depth-units
		GLX_STENCIL_SIZE, 8, // At least 8-bits in stencil-units
		
		GLX_RENDER_TYPE, GLX_RGBA_BIT, // Can be bound to RGBA contexts
		GLX_X_RENDERABLE, True, // Has an associated X visual
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR, // Associated x visual is of true-color type
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT, // Is able to draw to windows
		GLX_TRANSPARENT_TYPE, GLX_NONE, // Is opaque

		None // End of list
	};

	int configCount;
	GLXFBConfig *configs = glXChooseFBConfig(dis, scr, GLXConfigAttribs, &configCount);
	if (!configs) {
		XCloseDisplay(dis);
		
		throw log_except("Couldn't get framebuffer configurations!");
	}

	// TODO: Go through configs to find the best one?
	//       For now, use first available configuration
	XVisualInfo *vis = glXGetVisualFromFBConfig(dis, configs[0]);
	if (!vis) {
		XFree(configs);
		XCloseDisplay(dis);

		throw log_except("Couldn't get a visual from the configuration!");
	}

	// Create colormap
	cmap = XCreateColormap(dis, root, vis->visual, AllocNone);
	if (!cmap) {
		XFree(configs);
		XFree(vis);
		XCloseDisplay(dis);

		throw log_except("Couldn't create colormap!");
	}

	// Create window
	// Default value for border_pixmap is CopyFromParent, which fails if the depth
	// of our window is not the depth of the root window, howeer if border_pixel is
	// set, it overrides border_pixmap, and if we set the border size to 0, there
	// will be no border, so the value of border_pixel doesn't matter
	XSetWindowAttributes attribs;
	attribs.colormap = cmap;
	attribs.border_pixel = 0;
	attribs.event_mask =
    KeyPressMask|KeyReleaseMask|
    ButtonPressMask|ButtonReleaseMask|PointerMotionMask|
    StructureNotifyMask;

	win = XCreateWindow(dis, root, // Display and parent window
						0, 0, // Position of window, usually ignored
						800, 600, // Size of window, 800x600 by default
						0, vis->depth, // Border size and depth of window
						InputOutput, // Window class
						vis->visual, // Visual
						CWColormap | CWBorderPixel | // Attributes mask
						CWEventMask,
						&attribs); // Attributes

	XFree(vis);
	if (!win) {
		XFree(configs);
		XFreeColormap(dis, cmap);
		XCloseDisplay(dis);

		throw log_except("Couldn't create window!");
	}

	// Get GLX extension functions
	PFNGLXCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB =
		(PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
	if (!CreateContextAttribsARB) {
		XDestroyWindow(dis, win);
		XFree(configs);
		XFreeColormap(dis, cmap);
		XCloseDisplay(dis);

		throw log_except("Couldn't get glXCreateContextAttribsARB!");
	}

	// Create GLX context

	// Version 3.3, core profile
	static const int GLXContextAttribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 3,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

	ctx = CreateContextAttribsARB(dis, configs[0], NULL, True, GLXContextAttribs);
	if (!ctx) {
		XDestroyWindow(dis, win);
		XFree(configs);
		XFreeColormap(dis, cmap);
		XCloseDisplay(dis);

		throw log_except("Couldn't create GLX context!");
	}

	XFree(configs);

	glXMakeCurrent(dis, win, ctx);

	// Tell window manager to close when
	// the X window button is pressed
	WM_DELETE_WINDOW = XInternAtom(dis, "WM_DELETE_WINDOW", False);
	Atom WM_PROTOCOLS = XInternAtom(dis, "WM_PROTOCOLS", False);
	XChangeProperty(dis, win,
					WM_PROTOCOLS, XA_ATOM, 32,
					PropModeAppend, (u8*)&WM_DELETE_WINDOW, 1);
}

linux_gl_window_t::x_t::~x_t() {
	// Destroy OpenGL context
	glXMakeCurrent(dis, None, NULL);
	glXDestroyContext(dis, ctx);

	// Destroy window and free colormap
	XDestroyWindow(dis, win);
	XFreeColormap(dis, cmap);

	// Close display
	XCloseDisplay(dis);
}

linux_gl_window_t::linux_gl_window_t(window_init_t &init) :
	m_m(init.m), m_i(init.i), m_gl(m_i.mem, 800, 600)
{
	// Enable detectable autorepeat
	Bool supported;
	XkbSetDetectableAutoRepeat(x.dis, true, &supported);

	if (!supported) throw log_except("DetectableAutoRepeat not supported by x server!");
	
	// Make keycode map
	XDisplayKeycodes(x.dis, &m_minCode, &m_maxCode);
	m_map = XGetKeyboardMapping(x.dis, m_minCode, m_maxCode-m_minCode + 1, &m_symPerCode);

	// Show window
	XMapWindow(x.dis, x.win);
}

linux_gl_window_t::~linux_gl_window_t() {
	// Hide window
	XUnmapWindow(x.dis, x.win);

	// Free keycode map
	XFree(m_map);

	// Disable detectable autorepeat
	XkbSetDetectableAutoRepeat(x.dis, false, NULL);
}

// TODO: I should separate this into it's own file at some point
// Rate manager
class rate_t {
private:
	countTimer_t &m_t;
	
	// Time at which last frame was processed
	u64 m_lastFrame;

	// Counts between each occurrence
	u64 m_freq;

	// Inverse of m_freq
	f32 m_freqInv;

public:
	FINLINE rate_t(countTimer_t &t, u64 rate) : m_t(t) {
		// Round to nearest
		m_freq = (t.resolution() + (rate>>1))/rate;
		m_freqInv = 1.f/(f32)m_freq;
		m_lastFrame = 0;
	}

	// Have freq counts passed since last frame?
	FINLINE ubool ready() {
		const u64 t = m_t.time();
		if (t-m_lastFrame >= m_freq) {
			m_lastFrame = t;
			return true;
		}

		return false;
	}

	FINLINE f32 delta() {
		// Divide by m_freq
		return (f32)(m_t.time()-m_lastFrame) * m_freqInv;
	}
};

// X11 OpenGL main loop
window_loop_ret_t linux_gl_window_t::loop(game_t &game) {
	XEvent evt;
	rate_t framerate(m_i.timer, 60);
	unsigned int lastPressed = 0; // Last keycode pressed, detects autorepeat
  u32 width, height;

  XWindowAttributes attribs;
  XGetWindowAttributes(x.dis, x.win, &attribs);
  game.wstate().width = width = attribs.width;
  game.wstate().height = height = attribs.height;

  // For ButtonPress and ButtonRelease events
  static const key_code_t mouseKeys[] = {
    KEYC_NONE, // Buttons start at 1
    KEYC_M_PRIMARY, // Left click (yeah I'm assuming you're right handed)
    KEYC_M_MIDDLE, // Middle click
    KEYC_M_SECONDARY, // Right click
    KEYC_M_SCROLLUP, // Mouse wheel up
    KEYC_M_SCROLLDOWN, // Mouse wheel down
  };

	for (;;) {
		while (XPending(x.dis)) {
			XNextEvent(x.dis, &evt);

			switch (evt.type) {
			case ClientMessage:
				if ((Atom)evt.xclient.data.l[0] == x.WM_DELETE_WINDOW) return WINDOW_LOOP_SUCCESS;

				break;

			case KeyPress:
				// Detect autorepeat
				if (lastPressed == evt.xkey.keycode) break;
				lastPressed = evt.xkey.keycode;
				
				// Loop through keysyms for keycode
				for (const KeySym
               *kEnd = m_map + (evt.xkey.keycode-m_minCode + 1)*m_symPerCode,
               *k = kEnd - m_symPerCode;
             k != kEnd; ++k)
				{
					key_code_t c = getCodeFromSym(*k);

					if (c != KEYC_NONE) m_i.input.k.press(c);
				}

				break;

			case KeyRelease:
				// Autorepeat detection
				if (evt.xkey.keycode == lastPressed) lastPressed = 0;
				
				for (const KeySym
               *kEnd = m_map + (evt.xkey.keycode-m_minCode + 1)*m_symPerCode,
               *k = kEnd - m_symPerCode;
             k != kEnd; ++k)
				{
					key_code_t c = getCodeFromSym(*k);

					if (c != KEYC_NONE) m_i.input.k.release(c);
				}

        break;

      case MotionNotify:
        // TODO: Change this when aspect ratio correction comes into play!
        m_i.input.mx = evt.xmotion.x;
        m_i.input.my = evt.xmotion.y;
        break;

      case ButtonPress:
        // Press button as key
        m_i.input.k.press(mouseKeys[evt.xbutton.button]);
        break;

      case ButtonRelease:
        // Release button as key
        m_i.input.k.release(mouseKeys[evt.xbutton.button]);
        break;

      case ConfigureNotify:
        // Has the window changed size?
        if ((width != (u32)evt.xconfigure.width) ||
            (height != (u32)evt.xconfigure.height))
        {
          game.wstate().width = width = evt.xconfigure.width;
          game.wstate().height = height = evt.xconfigure.height;

          m_gl.resize(width, height);

          log_note("Resized");
        }

        break;

			default: break;
			}
		}

		if (framerate.ready()) {
			game_update_ret_t ret = game.update();
			m_i.input.k.update();

			switch (ret) {
			case GAME_UPDATE_CONTINUE: break;
			case GAME_UPDATE_RESET: return WINDOW_LOOP_RESET;
			case GAME_UPDATE_CLOSE: return WINDOW_LOOP_SUCCESS;
			case GAME_UPDATE_FAILED: return WINDOW_LOOP_FAILED;
			}

      // Check window state
      if ((game.wstate().width != width) || (game.wstate().height != height)) {
        // Resize window
        XWindowAttributes attrib;
        XGetWindowAttributes(x.dis, x.win, &attrib);

        XMoveResizeWindow(x.dis, x.win, attrib.x, attrib.y,
                          game.wstate().width, game.wstate().height);
      }

			if (!m_gl.render(game.rstate())) return WINDOW_LOOP_FAILED;
			glXSwapBuffers(x.dis, x.win);

			if (!m_m.audio->update()) return WINDOW_LOOP_FAILED;
		}
	}
}
