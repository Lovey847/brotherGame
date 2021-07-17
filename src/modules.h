#ifndef MODULES_H
#define MODULES_H

#include "types.h"

#include "window.h"
#include "audio.h"

// Struct containing all modules
struct modules_t {
	window_inst_t win;
	audio_inst_t audio;
};

#endif //MODULES_H
