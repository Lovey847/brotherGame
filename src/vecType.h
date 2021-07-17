// Included in vector.h, in vec4 class and ivec4 class
// Parameters:
//   VEC_TYPE: Name of vector type

// SIMD types
#ifdef PLAT_S_SSE2 // SSE2

__m128 pf;
__m128i pi;

#else // No SIMD

u64 u[2];

#endif // End of SIMD types

f32 f[4];
i32 i[4];

//////////////////
// Constructors

// No initialization
FINLINE VEC_TYPE() {}

// SIMD Constructors
#ifdef PLAT_S_SSE2 // SSE2

// Copy constructor
constexpr VEC_TYPE(const VEC_TYPE &other) :
	pf(other.pf) {}

constexpr VEC_TYPE(const __m128 &other) :
	pf(other) {}

constexpr VEC_TYPE(const __m128i &other) :
	pi(other) {}

#else // No SIMD

// Copy constructor
constexpr VEC_TYPE(const VEC_TYPE &other) :
	u{other.u[0], other.u[1]} {}

// u64 constructor
constexpr VEC_TYPE(const u64 other[2]) :
	u{other[0], other[1]} {}

constexpr VEC_TYPE(u64 x, u64 y) :
	u{x, y} {}

#endif // End of SIMD constructors

// SIMD operators
#ifdef PLAT_S_SSE2 // SSE2

FINLINE VEC_TYPE &operator=(const VEC_TYPE &other) {
	pf = other.pf;
	return *this;
}

#else // No SIMD

FINLINE VEC_TYPE &operator=(const VEC_TYPE &other) {
	u[0] = other.u[0];
	u[1] = other.u[1];

	return *this;
}

#endif // End of SIMD operators

// Math operators
FINLINE VEC_TYPE &operator+=(const VEC_TYPE &other) {
	return *this = *this + other;
}
FINLINE VEC_TYPE &operator-=(const VEC_TYPE &other) {
	return *this = *this - other;
}
FINLINE VEC_TYPE &operator*=(const VEC_TYPE &other) {
	return *this = *this * other;
}
FINLINE VEC_TYPE &operator/=(const VEC_TYPE &other) {
	return *this = *this / other;
}

FINLINE VEC_TYPE &operator|=(const VEC_TYPE &other) {
	return *this = *this | other;
}
FINLINE VEC_TYPE &operator&=(const VEC_TYPE &other) {
	return *this = *this & other;
}
FINLINE VEC_TYPE &operator^=(const VEC_TYPE &other) {
	return *this = *this ^ other;
}
FINLINE VEC_TYPE &operator<<=(i32 other) {
	return *this = *this << other;
}
FINLINE VEC_TYPE &operator>>=(i32 other) {
	return *this = *this >> other;
}

FINLINE VEC_TYPE operator|(i32 other) {
	return *this | VEC_TYPE(other);
}
FINLINE VEC_TYPE operator&(i32 other) {
	return *this & VEC_TYPE(other);
}
FINLINE VEC_TYPE operator^(i32 other) {
	return *this ^ VEC_TYPE(other);
}

FINLINE VEC_TYPE &operator|=(i32 other) {
	return *this = *this | other;
}
FINLINE VEC_TYPE &operator&=(i32 other) {
	return *this = *this & other;
}
FINLINE VEC_TYPE &operator^=(i32 other) {
	return *this = *this ^ other;
}

// More operators stored below in vector.h
