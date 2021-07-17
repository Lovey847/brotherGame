#ifndef TYPES_H
#define TYPES_H

#include "config.h"

#include <cstddef>
#include <cfloat>
#include <cstdint>
#include <climits>

// Fastest types
typedef int_fast8_t ifast;
typedef uint_fast8_t ufast;

// Signed sized types
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

// Unsigned sized types
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Pointer-width types
#if defined(INTPTR_MAX) && defined(UINTPTR_MAX)
typedef intptr_t iptr;
typedef uintptr_t uptr;
#else
typedef ptrdiff_t iptr;
typedef size_t uptr;
#endif

static constexpr uptr PTR_SIZE = sizeof(uptr);
static constexpr uptr PTR_BITS = sizeof(uptr)*CHAR_BIT;

// Floating-point types
// NOTE: This program fails to run if floating point bit representation
//       doesn't correspond to the IEEE-754 standard!
#if FLT_MAX_EXP == 128
typedef float f32;
#else
#	error Unknown 32-bit floating point type!
#endif

#if LDBL_MAX_EXP == 1024
typedef long double f64;
#elif DBL_MAX_EXP == 1024
typedef double f64;
#endif

// Boolean types
typedef ifast ibool;
typedef ufast ubool;

// FINLINE - forces inline
// Usage:
//     static FINLINE void func() {}

#ifdef NDEBUG // Only force inline in release build

#if defined(PLAT_C_GNU) // Any compiler with GNU extensions
#define FINLINE inline __attribute__((always_inline))

#elif defined(PLAT_C_MSVC) // MSVC
#define FINLINE __forceinline

#else // Unknown compiler, can't force inline
#define FINLINE inline
#endif

#else // Don't force inline in debug build
#define FINLINE inline

#endif
// End FINLINE

// Vector types, separated since the file is massive
// Includes many operations on many vector types, as well as
// SIMD support
#include "vector.h"

// Test types, to make sure they're as expected
static_assert(sizeof(i8) == 1, "");
static_assert(sizeof(i16) == 2, "");
static_assert(sizeof(i32) == 4, "");
static_assert(sizeof(i64) == 8, "");

static_assert(sizeof(u8) == 1, "");
static_assert(sizeof(u16) == 2, "");
static_assert(sizeof(u32) == 4, "");
static_assert(sizeof(u64) == 8, "");

static_assert(sizeof(f32) == 4, "");
static_assert(sizeof(f64) == 8, "");

static_assert(sizeof(vec4) == 16, "");

#endif //TYPES_H
