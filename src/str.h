// Various string utilities

#ifndef STR_H
#define STR_H

#include "types.h"
#include "util.h"
#include "endianUtil.h"

#include <string.h>

// endian-independent hash type, will always be in little endian
typedef u32 str_hash_t;

//////////////////////////////
// constexpr strlen
template<uptr N>
static constexpr uptr str_clen(const char (&)[N]) {
	return N-1;
}

//////////////////////////////////////
// Polynomial rolling hash function

// Temporary constants
#define P 127
#define M 2147483647

// Generate string hash at compile time
// String hash iteration function
template<uptr N>
static constexpr u64 str_hash_iter(const char (&s)[N], uptr i, u64 pow, u64 curHash) {
	return (i<str_clen(s) && s[i]) // Check for NULL terminator
			 ? str_hash_iter(s, i+1, (pow*P) % M, (curHash + (u64)s[i] * pow) % M)
		     : curHash;
}

// String hash function, computed at compile time
#define str_hash(s) (util_constexpr<str_hash_t, endian_little32((u32)str_hash_iter((s), 0, 1, 0))>())

// String hash function, computed at runtime
// ONLY CALL WHEN PERFORMANCE DOESN'T MATTER
str_hash_t str_hashR(const char *str);

#undef P
#undef M

////////////////////////////////////////////
// Conversions between numbers and strings

// Base string-integer conversion routine, returns in 64-bit unsigned integer
//
// Since the return value can be any number, I've decided to throw an error through
// the err ubool pointer
// err is optional and can be NULL
//
// If err isn't NULL, it's initial value determines how the function handles error reporting
//  If err is true, then when the function fails, the error is logged
//  If err is false, then when the function fails, no error is logged
u64 str_strnum_base(const char *str, uptr len, ubool *err = NULL);

// String-integer conversion
template<typename T>
static FINLINE T str_strnum(const char *str, ubool *err = NULL) {
	return (T)str_strnum_base(str, strlen(str), err);
}

// String-integer conversion, with default value in case there was an error
template<typename T>
static FINLINE T str_strnum_def(const char *str, T def) {
	ubool err = false;
	const u64 ret = str_strnum_base(str, strlen(str), &err);

	return err ? def : (T)ret;
}

// Base integer-string conversion routine, takes signed 64-bit integer as input
// Returns string length
// Returns 0 on failure
//
// If log is true, and 0 is returned, the error is logged
uptr str_numstr_base(char *out, i64 val, ubool log = true);

// Integer-string conversion
template<typename T>
static FINLINE uptr str_numstr(char *out, T val, ubool log = true) {
	return str_numstr_base(out, val, log);
}

// Integer-string conversion, with default value if str_numstr_base fails
template<typename T, uptr N>
static FINLINE uptr str_numstr_def(char *out, T val, const char (&def)[N]) {
	uptr ret = str_numstr_base(out, val);
	
	if (!ret) {
		strcpy(out, def);
		return str_clen(def);
	}
	
	return ret;
}

// Default string with unknown length
template<typename T>
static FINLINE uptr str_numstr_def(char *out, T val, const char *def) {
	uptr ret = str_numstr_base(out, val, false);

	if (!ret) {
		strcpy(out, def);
		return strlen(def);
	}

	return ret;
}

#endif //STR_H
