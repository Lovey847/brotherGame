#ifndef RNG_H
#define RNG_H

#include "types.h"

class rng_t {
private:
	u64 s;

public:
	FINLINE rng_t() {randomize();}

	// Randomize the RNG seed
	void randomize();

	// Set the RNG seed
	FINLINE void set(u64 seed) {s = seed;}

	// Get random value from s
	FINLINE u64 rand_base() {
		s ^= s<<13;
		s ^= s>>7;
		s ^= s<<17;

		return s;
	}

	template<typename T>
	FINLINE T rand() {
		static_assert(sizeof(T) <= sizeof(u64), "Type is too big!");

		union {
			u64 u;
			T t;
		} ret;

		ret.u = rand_base();
		return ret.t;
	}
};

#endif //RNG_H
