#ifndef ENDIANUTIL_H
#define ENDIANUTIL_H

#include "types.h"
#include "config.h"

// endianness symbols
#ifdef PLAT_E_LITTLE

static constexpr ubool endian_little = true;
static constexpr ubool endian_big = false;

#else //PLAT_E_LITTLE

static constexpr ubool endian_little = false;
static constexpr ubool endian_big = true;

#endif //PLAT_E_LITTLE

//// OPERATIONS //////////////////////////////////////////////////////
//   endian_swap: Swap endianness (INTEGERS ONLY)                   //
//   endian_swapMem: Swap endianness of value in memory             //
//   endian_swapMulti: Swap endianness of multiple values in memory //
//////////////////////////////////////////////////////////////////////

//// CATEGORIES ///////////////////////////////////////////////////////////////////////////
//   endian_little: Swap endianness if on big endian architecture                        //
//   endian_big: Swap endianness if on little endian architecture                        //
//   endian_cvt: Swap from little endian to machine endianness (alias for endian_little) //
///////////////////////////////////////////////////////////////////////////////////////////

// endian_swap
static constexpr u32 endian_swap32(u32 num) {
	return
		((num << 24)           ) |
		((num <<  8)&0x00ff0000) |
		((num >>  8)&0x0000ff00) |
		((num >> 24)           );
}

static constexpr i32 endian_swap32i(i32 num) {
	return (i32)endian_swap32((u32)num);
}

static constexpr u16 endian_swap16(u16 num) {
	return ((num<<16) | (num>>16));
}

static constexpr i16 endian_swap16i(i16 num) {
	return (i16)endian_swap16((u16)num);
}

// endian_swapMem
static FINLINE void endian_swapMem32(void *mem) {
	u32 * const mem32 = (u32*)mem;
	*mem32 = endian_swap32(*mem32);
}

static FINLINE void endian_swapMem16(void *mem) {
	u16 * const mem16 = (u16*)mem;
	*mem16 = endian_swap16(*mem16);
}

// endian_swapMulti
template<uptr len>
static FINLINE void endian_swapMulti32(void *mem) {
	static_assert(len < 65535, "I am almost positive you don't want that.");
	
	endian_swapMem32(mem);
	endian_swapMulti32<len-1>((u32*)mem+1);
}
template<> FINLINE void endian_swapMulti32<0>(void *mem) {(void)mem;}

template<uptr len>
static FINLINE void endian_swapMulti16(void *mem) {
	static_assert(len < 65535, "I am almost positive you don't want that.");
	
	endian_swapMem16(mem);
	endian_swapMulti16<len-1>((u16*)mem+1);
}
template<> FINLINE void endian_swapMulti16<0>(void *mem) {(void)mem;}

#ifdef PLAT_E_LITTLE
	// endian_little
	static constexpr u32 endian_little32(u32 num) {return num;}
	static constexpr i32 endian_little32i(i32 num) {return num;}
	static constexpr u16 endian_little16(u16 num) {return num;}
	static constexpr i16 endian_little16i(i16 num) {return num;}
	
	// endian_big
	static constexpr u32 endian_big32(u32 num) {return endian_swap32(num);}
	static constexpr i32 endian_big32i(i32 num) {return endian_swap32(num);}
	static constexpr u32 endian_big16(u16 num) {return endian_swap16(num);}
	static constexpr i32 endian_big16i(i16 num) {return endian_swap16(num);}
	
	// endian_little/bigMem
	static FINLINE void endian_littleMem32(void *mem) {(void)mem;}
	static FINLINE void endian_littleMem16(void *mem) {(void)mem;}
	static FINLINE void endian_bigMem32(void *mem) {endian_swapMem32(mem);}
	static FINLINE void endian_bigMem16(void *mem) {endian_swapMem16(mem);}
	
	// endian_little/bigMulti
	template<uptr len>
	static FINLINE void endian_littleMulti32(void *mem) {(void)mem;}
	template<uptr len>
	static FINLINE void endian_littleMulti16(void *mem) {(void)mem;}
	template<uptr len>
	static FINLINE void endian_bigMulti32(void *mem) {endian_swapMulti32<len>(mem);}
	template<uptr len>
	static FINLINE void endian_bigMulti16(void *mem) {endian_swapMulti32<len>(mem);}
#else //PLAT_E_LITTLE
	// endian_little
	static constexpr u32 endian_little32(u32 num) {return endian_swap32(num);}
	static constexpr i32 endian_little32i(i32 num) {return endian_swap32(num);}
	static constexpr u32 endian_little16(u16 num) {return endian_swap16(num);}
	static constexpr i32 endian_little16i(i16 num) {return endian_swap16(num);}
	
	// endian_big
	static constexpr u32 endian_big32(u32 num) {return num;}
	static constexpr i32 endian_big32i(i32 num) {return num;}
	static constexpr u16 endian_big16(u16 num) {return num;}
	static constexpr i16 endian_big16i(i16 num) {return num;}
	
	// endian_little/bigMem
	static FINLINE void endian_bigMem32(void *mem) {(void)mem;}
	static FINLINE void endian_bigMem16(void *mem) {(void)mem;}
	static FINLINE void endian_littleMem32(void *mem) {endian_swapMem32(mem);}
	static FINLINE void endian_littleMem16(void *mem) {endian_swapMem16(mem);}
	
	// endian_little/bigMulti
	template<uptr len>
	static FINLINE void endian_bigMulti32(void *mem) {(void)mem;}
	template<uptr len>
	static FINLINE void endian_bigMulti16(void *mem) {(void)mem;}
	template<uptr len>
	static FINLINE void endian_littleMulti32(void *mem) {endian_swapMulti32<len>(mem);}
	template<uptr len>
	static FINLINE void endian_littleMulti16(void *mem) {endian_swapMulti16<len>(mem);}
#endif //PLAT_E_LITTLE

// endian_cvt
static constexpr u32 endian_cvt32(u32 num) {return endian_big32(num);}
static constexpr i32 endian_cvt32(i32 num) {return endian_big32(num);}
static constexpr u16 endian_cvt16(u16 num) {return endian_big16(num);}
static constexpr i16 endian_cvt16(i16 num) {return endian_big16(num);}

// endian_cvtMem
static FINLINE void endian_cvtMem32(void *mem) {return endian_bigMem32(mem);}
static FINLINE void endian_cvtMem16(void *mem) {return endian_bigMem16(mem);}

// endian_cvtMulti
template<uptr len>
static FINLINE void endian_cvtMulti32(void *mem) {return endian_bigMulti32<len>(mem);}
template<uptr len>
static FINLINE void endian_cvtMulti16(void *mem) {return endian_bigMulti16<len>(mem);}

// Endian-independent types, mainly for data formats
// 16-bit version
template<typename T>
struct endian_type16_t {
	static_assert(sizeof(T) == 2, "");
	
	T little;
	T big;

#ifdef PLAT_E_LITTLE
	FINLINE operator T() const {return little;}

	FINLINE endian_type16_t &operator=(T other) {
		little = big = other;
		endian_swapMem16(&big);
		return *this;
	}
#else //PLAT_E_LITTLE
	FINLINE operator T() const {return big;}

	FINLINE endian_type16_t &operator=(T other) {
		little = big = other;
		endian_swapMem16(&little);
		return *this;
	}
#endif //PLAT_E_LITTLE
};

// 32-bit version
template<typename T>
struct endian_type32_t {
	static_assert(sizeof(T) == 4, "");
	
	T little;
	T big;

#ifdef PLAT_E_LITTLE
	FINLINE operator T() const {return little;}

	FINLINE endian_type32_t &operator=(T other) {
		little = big = other;
		endian_swapMem32(&big);
		return *this;
	}
#else
	FINLINE operator T() const {return big;}

	FINLINE endian_type32_t &operator=(T other) {
		little = big = other;
		endian_swapMem32(&little);
		return *this;
	}
#endif
};

typedef endian_type16_t<i16> endian_i16;
typedef endian_type32_t<i32> endian_i32;
typedef endian_type16_t<u16> endian_u16;
typedef endian_type32_t<u32> endian_u32;
typedef endian_type32_t<f32> endian_f32;

template<typename T>
struct endian_vec_t {
	static_assert(sizeof(T) == 16, "");
	
	T little;
	T big;

#ifdef PLAT_E_LITTLE
	FINLINE const T &v() const {return little;}
	FINLINE T &v() {return little;}

	FINLINE endian_vec_t &operator=(const T &other) {
		little = big = other;
		endian_swapMulti32<4>(&big);
		return *this;
	}
#else
	FINLINE const T &v() const {return big;}
	FINLINE T &v() {return big;}

	FINLINE endian_vec_t &operator=(const T &other) {
		little = big = other;
		endian_swapMutli32<4>(&little);
		return *this;
	}
#endif
};

typedef endian_vec_t<vec4> endian_vec4;
typedef endian_vec_t<vec2_2> endian_vec2_2;
typedef endian_vec_t<ivec4> endian_ivec4;
typedef endian_vec_t<ivec2_2> endian_ivec2_2;

#endif //ENDIANUTIL_H
