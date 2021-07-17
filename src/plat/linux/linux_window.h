#ifndef LINUX_WINDOW_H
#define LINUX_WINDOW_H

#include "types.h"
#include "window.h"
#include "modules.h"
#include "interfaces.h"
#include "game/game.h"

// Backend includes
#ifdef PLAT_B_OPENGL
#include "linux_gl_window.h"
#endif

// Window initialization parameters
struct window_init_t {
	modules_t &m;
	interfaces_t &i;

	game_t &g;
	
	FINLINE window_init_t(modules_t &mod, interfaces_t &inter, game_t &game) :
		m(mod), i(inter), g(game) {}
};

#endif //LINUX_WINDOW_H
