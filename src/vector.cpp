#include "types.h"

alignas(16) const u32 vec4_constants[] = {
	// VEC4_C_ZEROS
	0, 0, 0, 0,

	// VEC4_C_FULLMASK
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,

	// VEC4_C_TOPBIT
	0x80000000, 0x80000000, 0x80000000, 0x80000000,

	// VEC4_C_INTONE
	1, 1, 1, 1,

	// VEC4_C_FLTONE
	0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000,
};
