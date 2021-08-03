#include "types.h"
#include "endianUtil.h"
#include "args.h"
#include "str.h"
#include "game/game.h"

// Interfaces
// (base classes that have platform-defined member functions and/or
//  platform-defined derived classes that provide data necessary
//  to run it on said platform)
#include "mem.h"
#include "file.h"
#include "linux_file.h"
#include "interfaces.h"

// Modules
// (abstract virtual classes that inherit module_base_t,
//  and have platform-defined derived classes, and are instantiated
//  in the platform layer.
//  Basically interfaces with a selectable backend)
#include "module.h"
#include "window.h"
#include "linux_window.h"
#include "audio.h"
#include "modules.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

// Game loop, this is where all modules are initialized
// If a module fails to initialize, loop reports the error and tries another module backend
// If all backends of the module fail to initialize, loop reports the error and returns LOOP_FAILED
window_loop_ret_t loop(interfaces_t i, game_t &game) {
	modules_t m;

	// Catch any uncaught error and report it
	try {
		// Initialize window
		window_init_t winInit(m, i, game);

		if (m.win.setFallback((window_type_t)0, winInit) == WINDOW_COUNT) return WINDOW_LOOP_FAILED;

		// Initialize audio
		audio_init_t audioInit(m, i, game.state(), 44100);

		if (m.audio.setFallback((audio_type_t)0, audioInit) == AUDIO_COUNT) return WINDOW_LOOP_FAILED;

		// Run game loop
		return m.win->loop(game);
	} catch (const log_except_t &err) {
		log_warning("An uncaught error occurred: %s", err.str());
		return WINDOW_LOOP_FAILED;
	}
}

int main(int argc, char **argv) {
	// Initialize everything that stays between resets
  try {
    // Interfaces
    mem_t mem(32*1024*1024); // Allocate 32 mebibytes
    linux_file_system_t fileSys(mem);
    timer_t timer;
    args_t args(argc, argv, mem);
    game_input_t input;
    rng_t rng;
    interfaces_t inter(&mem, &fileSys, &timer, &args, &input, &rng);

    // Game
    game_t game(inter, args);

    // Start main loop
    window_loop_ret_t ret;
    while ((ret = loop(inter, game)) == WINDOW_LOOP_RESET);

    // 1 if true, 0 if false
    return ret == WINDOW_LOOP_FAILED;
  } catch (const log_except_t &err) {
    log_warning("FATAL ERROR: %s", err.str());
    return 1;
  }
}
