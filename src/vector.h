#ifndef VECTOR_H
#define VECTOR_H

/////////////////////////
// SIMD includes

// SSE & SSE2
#ifdef PLAT_S_SSE2 // SSE2

#include <xmmintrin.h>
#include <emmintrin.h>

// TODO: Move this out of SSE2 includes!
//
// Endian independent variants of endian dependent intrinsics
#ifdef PLAT_E_LITTLE

// xyzw is 4 hexadecimal digits, selecting the new order of
// coordinates.
//
// Examples:
//   0x0123 = xyzw
//   0x0312 = xwyz
//   0x1320 = ywzx
//   0x0012 = xxyz
//   0x2211 = zzyy
#define VEC4_SSE2_SHUFFLEMASK(xyzw) (			\
	(((xyzw)&0x3000) >> 12) |					\
	(((xyzw)&0x0300) >>  6) |					\
	(((xyzw)&0x0030)      ) |					\
	(((xyzw)&0x0003) <<  6)						\
)

// Since endianness effects which direction these will actually go,
// I define version that'll consistenly go where you want them to here
#define VEC4_SSE2_SRLI _mm_slli_si128
#define VEC4_SSE2_SLLI _mm_srli_si128

#define VEC4_SSE2_SET _mm_setr_ps
#define IVEC4_SSE2_SET _mm_setr_epi32

#else // Machine is big endian

#define VEC4_SSE2_SHUFFLEMASK(xyzw) (		\
	(((xyzw)&0x3000) >> 6) |				\
	(((xyzw)&0x0300) >> 4) |				\
	(((xyzw)&0x0030) >> 2) |				\
	(((xyzw)&0x0003)     )					\
)

#define VEC4_SSE2_SRLI _mm_srli_si128
#define VEC4_SSE2_SLLI _mm_slli_si128

#define VEC4_SSE2_SET _mm_set_ps
#define IVEC4_SSE2_SET _mm_set_epi32

#endif // Endian independent variants of endian dependent intrinsics

#else // No SIMD

#include <string.h>

#endif

// SIMD includes
//////////////////////

////////////////////////////////////
// Commonly used vector constants

// If a constant is used commonly enough throughout the code, I'll put it here
// to potentially speed up access to it

// I fill out vec4_constants in vector.cpp, describe it's contents here, and
// give pointers to vec4_constants's data
extern const u32 vec4_constants[];

enum vec4_constants_e {
	// Most of these are single values, but some of them are arrays
	// The ones that are arrays will start with "index from <start index> to <end index>" in their description

	// Operations in descriptions will be performed as if constants were 4d vectors
	
	VEC4_C_ZEROS = 0, // Raw bytes 0x00000000, 0x00000000, 0x00000000, 0x00000000
	VEC4_C_FULLMASK = 4, // Raw bytes 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	VEC4_C_TOPBIT = 8, // Raw bytes 0x80000000, 0x80000000, 0x80000000, 0x80000000
	VEC4_C_INTONE = 12, // Raw bytes 0x00000001, 0x00000001, 0x00000001, 0x00000001
	VEC4_C_FLTONE = 16, // Raw bytes 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000
};

// I define these since vec4 and ivec4 don't exist yet
// I put the constants before the unions since some of the member functions
// use the constants
#define vec4_zeros ((vec4*)(vec4_constants + VEC4_C_ZEROS))
#define vec4_fullMask ((vec4*)(vec4_constants + VEC4_C_FULLMASK))
#define vec4_topBit ((vec4*)(vec4_constants + VEC4_C_TOPBIT))
#define vec4_intOne ((vec4*)(vec4_constants + VEC4_C_INTONE))
#define vec4_fltOne ((vec4*)(vec4_constants + VEC4_C_FLTONE))

#define ivec4_zeros ((ivec4*)vec4_zeros)
#define ivec4_fullMask ((ivec4*)vec4_fullMask)
#define ivec4_topBit ((ivec4*)vec4_topBit)
#define ivec4_intOne ((ivec4*)vec4_intOne)
#define ivec4_fltOne ((ivec4*)vec4_fltOne)

// Commonly used vector constants
//////////////////////////////////////

//////////////////////
// Vector types

union alignas(16) vec4;
union alignas(16) ivec4;

// Integer initializer for vec4
union vec4_int_init {
	// SIMD members
#ifdef PLAT_S_SSE2 // SSE2

	__m128i p;
	
#else // No SIMD

	u64 u[2];
	
#endif // SIMD members
	
	i32 i[4];

	constexpr vec4_int_init(i32 val) :
		i{val, val, val, val} {}
	constexpr vec4_int_init(i32 x, i32 y, i32 z, i32 w) :
		i{x, y, z, w} {}
	constexpr vec4_int_init(const i32 arr[4]) :
		i{arr[0], arr[1], arr[2], arr[3]} {}
};

// "Constructor" for vec4 int initialization
#define vec4_int(...) (vec4(vec4_int_init(__VA_ARGS__)))

union alignas(16) vec4 {
#define VEC_TYPE vec4
#include "vecType.h"
#undef VEC_TYPE

	// Whole vector initialization
	constexpr vec4(f32 x, f32 y, f32 z, f32 w) :
		f{x, y, z, w} {}
	constexpr vec4(const f32 ff[4]) :
		f{ff[0], ff[1], ff[2], ff[3]} {}

	// Single value initialization
	constexpr vec4(f32 val) :
		f{val, val, val, val} {}

	// Integer initialization
#ifdef PLAT_S_SSE2 // SSE2
	
	constexpr vec4(const vec4_int_init &other) :
		pi(other.p) {}
	
#else // No SIMD
	
	constexpr vec4(const vec4_int_init &other) :
		u{other.u[0], other.u[1]} {}
	
#endif // Integer initialization

	// Casting operation
	FINLINE const ivec4 &ccast() const {
		return *(const ivec4*)this;
	}
	FINLINE ivec4 &cast() {
		return *(ivec4*)this;
	}

	// SIMD math operators
#ifdef PLAT_S_SSE2 // SSE2

	// Common math operators
	FINLINE vec4 operator+(const vec4 &right) const {
		return vec4(_mm_add_ps(pf, right.pf));
	}
	FINLINE vec4 operator-(const vec4 &right) const {
		return vec4(_mm_sub_ps(pf, right.pf));
	}
	FINLINE vec4 operator*(const vec4 &right) const {
		return vec4(_mm_mul_ps(pf, right.pf));
	}
	FINLINE vec4 operator/(const vec4 &right) const {
		return vec4(_mm_div_ps(pf, right.pf));
	}

	// Bitwise math operators
	FINLINE vec4 operator|(const vec4 &right) const {
		return vec4(_mm_or_ps(pf, right.pf));
	}
	FINLINE vec4 operator&(const vec4 &right) const {
		return vec4(_mm_and_ps(pf, right.pf));
	}
	FINLINE vec4 operator^(const vec4 &right) const {
		return vec4(_mm_xor_ps(pf, right.pf));
	}
	FINLINE vec4 operator<<(i32 right) const {
		return vec4(_mm_slli_epi32(pi, right));
	}
	FINLINE vec4 operator>>(i32 right) const {
		return vec4(_mm_srai_epi32(pi, right));
	}
	FINLINE vec4 operator~() const {
		return vec4(_mm_xor_ps(pf, vec4_fullMask->pf));
	}

	// Returns (~left)&right
	FINLINE vec4 andNot(const vec4 &right) const {
		return vec4(_mm_andnot_ps(pf, right.pf));
	}

	// Special vector routines

	// mask has four hexadecimal digits, the first represents x, second represents y,
	// third represents z, fourth represents w, each digit can be:
	// 0: x
	// 1: y
	// 2: z
	// 3: w
	template<u32 mask> FINLINE vec4 shuffle() const {
		return vec4(_mm_shuffle_ps(pf, pf, VEC4_SSE2_SHUFFLEMASK(mask)));
	}

	// Shift coordinates left and right, shifting in 0's (x being leftmost, and w being right most)
	template<u32 shift> FINLINE vec4 shrCoord() const {
		return vec4(VEC4_SSE2_SRLI(pi, shift*4));
	}
	template<u32 shift> FINLINE vec4 shlCoord() const {
		return vec4(VEC4_SSE2_SLLI(pi, shift*4));
	}
#else // No SIMD

	// Common math operators
	FINLINE vec4 operator+(const vec4 &right) const {
		return vec4(f[0] + right.f[0], f[1] + right.f[1], f[2] + right.f[2], f[3] + right.f[3]);
	}
	FINLINE vec4 operator-(const vec4 &right) const {
		return vec4(f[0] - right.f[0], f[1] - right.f[1], f[2] - right.f[2], f[3] - right.f[3]);
	}
	FINLINE vec4 operator*(const vec4 &right) const {
		return vec4(f[0] * right.f[0], f[1] * right.f[1], f[2] * right.f[2], f[3] * right.f[3]);
	}
	FINLINE vec4 operator/(const vec4 &right) const {
		return vec4(f[0] / right.f[0], f[1] / right.f[1], f[2] / right.f[2], f[3] / right.f[3]);
	}

	// Bitwise math operators
	FINLINE vec4 operator|(const vec4 &right) const {
		return vec4(u[0] | right.u[0], u[1] | right.u[1]);
	}
	FINLINE vec4 operator&(const vec4 &right) const {
		return vec4(u[0] & right.u[0], u[1] & right.u[1]);
	}
	FINLINE vec4 operator^(const vec4 &right) const {
		return vec4(u[0] | right.u[0], u[1] | right.u[1]);
	}
	FINLINE vec4 operator<<(i32 right) const {
		return vec4(i[0] << right, i[1] << right, i[2] << right, i[3] << right);
	}
	FINLINE vec4 operator>>(i32 right) const {
		return vec4(i[0] >> right, i[1] >> right, i[2] >> right, i[3] >> right);
	}
	FINLINE vec4 operator~() const {
		return vec4(~u[0], ~u[1]);
	}

	FINLINE vec4 andNot(const vec4 &other) {
		return (~*this)&other;
	}

	// Special vector routines
	
	// mask has four hexadecimal digits, the first represents x, second represents y,
	// third represents z, fourth represents w, each digit can be:
	// 0: x
	// 1: y
	// 2: z
	// 3: w
	template<u32 mask> FINLINE vec4 shuffle() const {
		return vec4(f[(mask&0x3000) >> 12],
					f[(mask&0x0300) >>  8],
					f[(mask&0x0030) >>  4],
					f[(mask&0x0003)      ]);
	}

	// Shift coordinates left and right, shifting in 0's (x being leftmost, and w being right most)
	template<u32 shift> FINLINE vec4 shrCoord() const {
		return vec4(shift     ? 0 : f[0      ], shift > 1 ? 0 : f[1-shift],
					shift > 2 ? 0 : f[2-shift], shift > 3 ? 0 : f[3-shift]);
	}
	template<u32 shift> FINLINE vec4 shlCoord() const {
		return vec4(shift > 3 ? 0 : f[0+shift], shift > 2 ? 0 : f[1+shift],
					shift > 1 ? 0 : f[2+shift], shift     ? 0 : f[3]);
	}
#endif // SIMD math operators

	// Extra math operators
	FINLINE vec4 operator+(f32 right) const {
		return *this + vec4(right);
	}
	FINLINE vec4 operator-(f32 right) const {
		return *this - vec4(right);
	}
	FINLINE vec4 operator*(f32 right) const {
		return *this * vec4(right);
	}
	FINLINE vec4 operator/(f32 right) const {
		return *this / vec4(right);
	}

	FINLINE vec4 &operator+=(f32 right) {
		return *this = *this + right;
	}
	FINLINE vec4 &operator-=(f32 right) {
		return *this = *this - right;
	}
	FINLINE vec4 &operator*=(f32 right) {
		return *this = *this * right;
	}
	FINLINE vec4 &operator/=(f32 right) {
		return *this = *this / right;
	}
	
	FINLINE vec4 operator-() const {
		return *this ^ *vec4_topBit;
	}
};

// Flaot initializer for ivec4
union ivec4_float_init {
	// SIMD members
#ifdef PLAT_S_SSE2 // SSE2

	__m128 p;
	
#else // No SIMD

	u64 u[2];
	
#endif // SIMD members
	
	f32 f[4];

	constexpr ivec4_float_init(f32 val) :
		f{val, val, val, val} {}
	constexpr ivec4_float_init(f32 x, f32 y, f32 z, f32 w) :
		f{x, y, z, w} {}
	constexpr ivec4_float_init(const f32 arr[4]) :
		f{arr[0], arr[1], arr[2], arr[3]} {}
};

// "Constructor" for ivec4 float initialization
#define ivec4_float(...) (ivec4(ivec4_float_init(__VA_ARGS__)))

// Signed-integer vector type
union alignas(16) ivec4 {
#define VEC_TYPE ivec4
#include "vecType.h"
#undef VEC_TYPE

	// Whole vector initialization
	constexpr ivec4(i32 x, i32 y, i32 z, i32 w) :
		i{x, y, z, w} {}
	constexpr ivec4(const i32 ii[4]) :
		i{ii[0], ii[1], ii[2], ii[3]} {}

	// Single value initialization
	constexpr ivec4(i32 val) :
		i{val, val, val, val} {}

	// Float initialization
#ifdef PLAT_S_SSE2 // SSE2

	constexpr ivec4(const ivec4_float_init &other) :
		pf(other.p) {}
	
#else // No SIMD

	constexpr ivec4(const ivec4_float_init &other) :
		u{other.u[0], other.u[1]} {}
	
#endif // Float initialization

	// Casting operation
	FINLINE const vec4 &ccast() const {
		return *(const vec4*)this;
	}
	FINLINE vec4 &cast() {
		return *(vec4*)this;
	}

	// SIMD math operators
#ifdef PLAT_S_SSE2 // SSE2 math operators
	
	// Common math operators
	FINLINE ivec4 operator+(const ivec4 &right) const {
		return ivec4(_mm_add_epi32(pi, right.pi));
	}
	FINLINE ivec4 operator-(const ivec4 &right) const {
		return ivec4(_mm_sub_epi32(pi, right.pi));
	}
	FINLINE ivec4 operator*(const ivec4 &right) const {
		// Multiply 2 4D vectors into 2 2D vectors
		const __m128i axz_bxz = _mm_mul_epu32(pi, right.pi);
		const __m128i ayw_byw = _mm_mul_epu32(VEC4_SSE2_SLLI(pi, 4), VEC4_SSE2_SLLI(right.pi, 4));

		// Pack 2 2D vectors into 1 4D vector and return
		return ivec4(_mm_unpacklo_epi32(_mm_shuffle_epi32(axz_bxz, VEC4_SSE2_SHUFFLEMASK(0x0200)),
										_mm_shuffle_epi32(ayw_byw, VEC4_SSE2_SHUFFLEMASK(0x0200))));
	}
	FINLINE ivec4 operator/(const ivec4 &right) const {
		// Convert integer vectors to floating-point vectors, divide, and convert result to an integer vector
		return ivec4(_mm_cvttps_epi32(_mm_div_ps(_mm_cvtepi32_ps(pi), _mm_cvtepi32_ps(right.pi))));
	}

	// Bitwise math operators
	FINLINE ivec4 operator|(const ivec4 &right) const {
		return ivec4(_mm_or_si128(pi, right.pi));
	}
	FINLINE ivec4 operator&(const ivec4 &right) const {
		return ivec4(_mm_and_si128(pi, right.pi));
	}
	FINLINE ivec4 operator^(const ivec4 &right) const {
		return ivec4(_mm_xor_si128(pi, right.pi));
	}
	FINLINE ivec4 operator<<(i32 right) const {
		return ivec4(_mm_slli_epi32(pi, right));
	}
	FINLINE ivec4 operator>>(i32 right) const {
		return ivec4(_mm_srli_epi32(pi, right));
	}
	FINLINE ivec4 operator~() const {
		return ivec4(_mm_xor_si128(pi, ivec4_fullMask->pi));
	}

	// Returns (~left)&right
	FINLINE ivec4 andNot(const ivec4 &right) const {
		return ivec4(_mm_andnot_si128(pi, right.pi));
	}

	// Special vector routines

	// mask has four hexadecimal digits, the first represents x, second represents y,
	// third represents z, fourth represents w, each digit can be:
	// 0: x
	// 1: y
	// 2: z
	// 3: w
	template<u32 mask> FINLINE ivec4 shuffle() const {
		return ivec4(_mm_shuffle_epi32(pi, VEC4_SSE2_SHUFFLEMASK(mask)));
	}

	// Shift coordinates left and right, shifting in 0's (x being leftmost, and w being right most)
	template<u32 shift> FINLINE ivec4 shrCoord() const {
		return ivec4(VEC4_SSE2_SRLI(pi, shift*4));
	}
	template<u32 shift> FINLINE ivec4 shlCoord() const {
		return ivec4(VEC4_SSE2_SLLI(pi, shift*4));
	}
#else // No SIMD
	
	// Common math operators
	FINLINE ivec4 operator+(const ivec4 &right) const {
		return ivec4(i[0] + right.i[0], i[1] + right.i[1], i[2] + right.i[2], i[3] + right.i[3]);
	}
	FINLINE ivec4 operator-(const ivec4 &right) const {
		return ivec4(i[0] - right.i[0], i[1] - right.i[1], i[2] - right.i[2], i[3] - right.i[3]);
	}
	FINLINE ivec4 operator*(const ivec4 &right) const {
		return ivec4(i[0] * right.i[0], i[1] * right.i[1], i[2] * right.i[2], i[3] * right.i[3]);
	}
	FINLINE ivec4 operator/(const ivec4 &right) const {
		return ivec4(i[0] / right.i[0], i[1] / right.i[1], i[2] / right.i[2], i[3] / right.i[3]);
	}

	// Bitwise math operators
	FINLINE ivec4 operator|(const ivec4 &right) const {
		return ivec4(u[0] | right.u[0], u[1] | right.u[1]);
	}
	FINLINE ivec4 operator&(const ivec4 &right) const {
		return ivec4(u[0] & right.u[0], u[1] & right.u[1]);
	}
	FINLINE ivec4 operator^(const ivec4 &right) const {
		return ivec4(u[0] ^ right.u[0], u[1] ^ right.u[1]);
	}
	FINLINE ivec4 operator<<(i32 right) const {
		return ivec4(i[0] << right, i[1] << right, i[2] << right, i[3] << right);
	}
	FINLINE ivec4 operator>>(i32 right) const {
		return ivec4(i[0] >> right, i[1] >> right, i[2] >> right, i[3] >> right);
	}
	FINLINE ivec4 operator~() const {
		return ivec4(~i[0], ~i[1], ~i[2], ~i[3]);
	}

	FINLINE ivec4 andNot(const ivec4 &other) {
		return (~*this)&other;
	}

	// Special vector routines
	
	// mask has four hexadecimal digits, the first represents x, second represents y,
	// third represents z, fourth represents w, each digit can be:
	// 0: x
	// 1: y
	// 2: z
	// 3: w
	template<u32 mask> FINLINE ivec4 shuffle() const {
		return ivec4(i[(mask&0x3000) >> 12],
					 i[(mask&0x0300) >>  8],
					 i[(mask&0x0030) >>  4],
					 i[(mask&0x0003)      ]);
	}

	// Shift coordinates left and right, shifting in 0's (x being leftmost, and w being right most)
	template<u32 shift> FINLINE ivec4 shrCoord() const {
		return ivec4(shift     ? 0 : i[0      ], shift > 1 ? 0 : i[1-shift],
					 shift > 2 ? 0 : i[2-shift], shift > 3 ? 0 : i[3-shift]);
	}
	template<u32 shift> FINLINE ivec4 shlCoord() const {
		return ivec4(shift > 3 ? 0 : i[0+shift], shift > 2 ? 0 : i[1+shift],
					 shift > 1 ? 0 : i[2+shift], shift     ? 0 : i[3]);
	}
#endif // SIMD math operators

	// Extra math operators
	FINLINE ivec4 operator+(i32 right) const {
		return *this + ivec4(right);
	}
	FINLINE ivec4 operator-(i32 right) const {
		return *this - ivec4(right);
	}
	FINLINE ivec4 operator*(i32 right) const {
		return *this * ivec4(right);
	}
	FINLINE ivec4 operator/(i32 right) const {
		return *this / ivec4(right);
	}
	
	FINLINE ivec4 &operator+=(i32 right) {
		return *this = *this + right;
	}
	FINLINE ivec4 &operator-=(i32 right) {
		return *this = *this - right;
	}
	FINLINE ivec4 &operator*=(i32 right) {
		return *this = *this * right;
	}
	FINLINE ivec4 &operator/=(i32 right) {
		return *this = *this / right;
	}
	
	FINLINE ivec4 operator-() const {
		return (~*this) + *ivec4_intOne;
	}
};

// 2 2-dimensional vectors that are stored in 16 bytes
typedef vec4 vec2_2;
typedef ivec4 ivec2_2;

// Vector types
///////////////////

////////////////////////////////////////////////////
// Vector initialization and destruction routines

// Run this at the start of main()
static FINLINE void vec_init() {
}

// Run this at the end of main()
static FINLINE void vec_uninit() {
}

// Vector initialization and destruction routines
////////////////////////////////////////////////////

//////////////////////////
// Conversion functions

// SIMD conversion functions
#ifdef PLAT_S_SSE2 // SSE2

static FINLINE vec4 vec4_ivec4(const ivec4 &other) {
	return vec4(_mm_cvtepi32_ps(other.pi));
}

static FINLINE ivec4 ivec4_vec4(const vec4 &other) {
	return ivec4(_mm_cvttps_epi32(other.pf));
}

#else // No SIMD

static FINLINE vec4 vec4_ivec4(const ivec4 &other) {
	return vec4(other.i[0], other.i[1], other.i[2], other.i[3]);
}

static FINLINE ivec4 ivec4_vec4(const vec4 &other) {
	return ivec4(other.f[0], other.f[1], other.f[2], other.f[3]);
}

#endif // SIMD conversion functions

// Conversion functions
///////////////////////////

#endif //VECTOR_H
