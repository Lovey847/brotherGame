#ifndef INTERFACES_H
#define INTERFACES_H

#include "types.h"

#include "mem.h"
#include "file.h"
#include "countTimer.h"
#include "args.h"
#include "game/input.h"
#include "rng.h"

// Struct containing a reference to all interfaces
// A class that's either initialized, or managed, by the platform layer
// constitutes as an interface
struct interfaces_t {
	mem_t &mem;
	file_system_t &fileSys;
	countTimer_t &timer;
	args_t &args;
	game_input_t &input;
	rng_t &rng;

	constexpr interfaces_t(void *memp, void *fileSysp, void *timerp, void *argsp, void *inputp, void *rngp) :
		mem(*(mem_t*)memp), fileSys(*(file_system_t*)fileSysp), timer(*(countTimer_t*)timerp),
		args(*(args_t*)argsp), input(*(game_input_t*)inputp), rng(*(rng_t*)rngp) {}
};

#endif //INTERFACES_H
