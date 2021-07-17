#include "types.h"
#include "rng.h"

#include <ctime>

void rng_t::randomize() {
	// Multiply time() by a random value
	s = (u64)time(NULL) * 0x547e5fb6;
}
