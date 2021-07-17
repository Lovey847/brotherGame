#ifndef WINDOW_H
#define WINDOW_H

#include "types.h"
#include "module.h"
#include "game/game.h"
#include "log.h"

// Window module backend type
enum window_type_t : ufast {
#ifdef PLAT_B_OPENGL
	WINDOW_OPENGL,
#endif

	WINDOW_COUNT
};

// Make sure this module has backends
static_assert(WINDOW_COUNT >= 1, "");

// Window loop return value
enum window_loop_ret_t : ufast {
	WINDOW_LOOP_RESET = 0, // Reset main loop with new settings for modules
	WINDOW_LOOP_SUCCESS, // Main loop has returned successfully
	WINDOW_LOOP_FAILED, // Main loop has encountered an error and couldn't proceed with execution
};

// Window init struct, platform dependent
struct window_init_t;

static constexpr uptr WINDOW_MAXSIZE = 1024;
class window_base_t : public module_base_t<WINDOW_MAXSIZE> {
public:
	// Start main loop
	// Window should get everything it needs from window_init_t
	virtual window_loop_ret_t loop(game_t &game) = 0;
	virtual ~window_base_t() {
		log_note("Destructing window module");
	}
};

// Backend constructor table
extern const module_constructProc_t<window_base_t, window_init_t> window_construct[WINDOW_COUNT];

// Put in platform layer to instantiate the window
typedef module_inst_t<window_base_t, window_type_t, WINDOW_COUNT,
					  window_init_t, WINDOW_MAXSIZE, window_construct> window_inst_t;

#endif //WINDOW_H
