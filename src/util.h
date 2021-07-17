// Miscellaneous utilities

#ifndef UTIL_H
#define UTIL_H

#include "types.h"
#include "endianUtil.h"
#include "log.h"

// Integer value alignment
template<typename T>
constexpr T util_alignDown(T x, T alignment) {
	return x - x%alignment;
}

template<typename T>
constexpr T util_alignRound(T x, T alignment) {
	return util_alignDown(x+alignment/2, alignment);
}

template<typename T>
constexpr T util_alignUp(T x, T alignment) {
	return util_alignDown(x+alignment-1, alignment);
}

// Integer rounded division
template<typename T>
constexpr T util_divRound(T x, T y) {
	return (x + y/2) / y;
}

template<typename T>
constexpr T util_divUp(T x, T y) {
	return (x + y-1) / y;
}

// Endian-independent magic-code creation
constexpr u32 util_magic(u32 a, u32 b, u32 c, u32 d) {
	return endian_little32(a | (b << 8) | (c << 16) | (d << 24));
}

// Swapping function
template<class T1, class T2>
void util_swap(T1 &a, T2 &b) {
	T1 tmp = a;
	a = b;
	b = tmp;
}

// Min and max function
template<class T>
constexpr T util_min(T a, T b) {
	return (a < b) ? a : b;
}

template<class T>
constexpr T util_max(T a, T b) {
	return (a > b) ? a : b;
}

// Clamp function
template<class T>
constexpr T util_clamp(T v, T min, T max) {
	return util_min(util_max(v, max), min);
}

// Alignment mask
#define util_alignmask(type) (alignof(type)-1)

// Size of array
#define util_arrlen(arr) (sizeof(arr)/sizeof(arr[0]))

// Force constexpr evaluation
template<typename T, T ret>
static constexpr T util_constexpr() {return ret;}

// Percentage of integer
template<typename T, uptr cap = 65536>
static constexpr T util_percent(T val, T percent) {
	return val*percent/cap;
}

#endif //UTIL_H
