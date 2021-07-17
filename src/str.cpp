#include "types.h"
#include "util.h"
#include "endianUtil.h"
#include "str.h"
#include "log.h"

#include <cstring>

#define P 127
#define M 2147483647

str_hash_t str_hashR(const char *str) {
	u64 curHash, pow;

	curHash = 0;
	pow = 1;
	for (const char *i = str; *i; ++i) {
		curHash = (curHash + (u64)*i*pow) % M;
		pow = (pow*P) % M;
	}

	return endian_little32((u32)curHash);
}

#undef P
#undef M

// Table containing powers of 10
static const u64 pow10[] = {
	1U,
	10U,
	100U,
	1000U,
	10000U,
	100000U,
	1000000U,
	10000000U,
	100000000U,
	1000000000U,
	10000000000U,
	100000000000U,
	1000000000000U,
	10000000000000U,
	100000000000000U,
	1000000000000000U,
	10000000000000000U,
	100000000000000000U,
	1000000000000000000U,
	10000000000000000000U
};

u64 str_strnum_base(const char *str, uptr len, ubool *err) {
	// Parameter sanitization
	if (!str) {
		if (!err || *err) log_warning("str is NULL!");
		else if (err) *err = true;
		
		return 0;
	}
	if (len >= util_arrlen(pow10)) {
		if (!err || *err) log_warning("%s is too big!", str);
		else if (err) *err = true;
		
		return 0;
	}
	
	u64 ret = 0;

	ubool negative = false;
	if (*str == '-') {
		++str;
		negative = true;
	}

#if 0 // Inline this loop
	
	while (len--) {
		u64 num = *(unsigned char*)str++ - '0';
		if (num >= 10) {
			if (!err || *err) log_warning("NaN!");
			else if (err) *err = true;
			
			return 0;
		}
		
		ret += pow10[len] * num;
	}

#else

#define ITER(val)								\
	num = *(u8*)str++ - '0';					\
	if (num >= 10) goto l_nan;					\
												\
	ret += (val)*num

	u64 num;
	switch (len) {
	case 20: ITER(10000000000000000000U); // fall through
	case 19: ITER(1000000000000000000U); // fall through
	case 18: ITER(100000000000000000U); // fall through
	case 17: ITER(10000000000000000U); // fall through
	case 16: ITER(1000000000000000U); // fall through
	case 15: ITER(100000000000000U); // fall through
	case 14: ITER(10000000000000U); // fall through
	case 13: ITER(1000000000000U); // fall through
	case 12: ITER(100000000000U); // fall through
	case 11: ITER(10000000000U); // fall through
	case 10: ITER(1000000000U); // fall through
	case 9:  ITER(100000000U); // fall through
	case 8:  ITER(10000000U); // fall through
	case 7:  ITER(1000000U); // fall through
	case 6:  ITER(100000U); // fall through
	case 5:  ITER(10000U); // fall through
	case 4:  ITER(1000U); // fall through
	case 3:  ITER(100U); // fall through
	case 2:  ITER(10U); // fall through
	case 1:  ITER(1U); break;
		
	case 0:  return 0;
	}

#undef ITER

#endif

	return negative ? -ret : ret;
	
l_nan:
	if (!err || *err) log_warning("NaN!");
	else if (err) *err = true;

	return 0;
}

uptr str_numstr_base(char *out, i64 val, ubool log) {
	uptr ret = 0;
	
	// Parameter sanitization
	if (!out) {
		if (log) log_warning("out is NULL!");
		return 0;
	}

	// Negative number handling
	u64 num = val;
	if (val < 0) {
		num = -val;
		*out++ = '-';
		++ret;
	}

	// Special case for 0
	if (!num) {
//		*(u16*)str = endian_little16('0'); // This has alignment issues
		
		out[0] = '0';
		out[1] = 0;
		return 1;
	}

	// Find number length
	// Since number is signed, 10 digits is impossible
	uptr len = util_arrlen(pow10)-2;
	while (pow10[len] > num) --len;
	ret += len+1;

	// Loop through number, writing characters to string

#if 0 // Inline this loop
	
	do {
		const u64 v = num/pow10[len];
		*out++ = '0' + num;
		num -= v;
	} while (len--);
	
#else

#define ITER(val)								\
	v = num/(val);								\
	*out++ = '0' + v;							\
	num -= v*(val)

	u64 v;
	switch (len) {
	case 18: ITER(1000000000000000000U); // fall through
	case 17: ITER(100000000000000000U); // fall through
	case 16: ITER(10000000000000000U); // fall through
	case 15: ITER(1000000000000000U); // fall through
	case 14: ITER(100000000000000U); // fall through
	case 13: ITER(10000000000000U); // fall through
	case 12: ITER(1000000000000U); // fall through
	case 11: ITER(100000000000U); // fall through
	case 10: ITER(10000000000U); // fall through
	case 9:  ITER(1000000000U); // fall through
	case 8:  ITER(100000000U); // fall through
	case 7:  ITER(10000000U); // fall through
	case 6:  ITER(1000000U); // fall through
	case 5:  ITER(100000U); // fall through
	case 4:  ITER(10000U); // fall through
	case 3:  ITER(1000U); // fall through
	case 2:  ITER(100U); // fall through
	case 1:  ITER(10U); // fall through
	case 0:  *out++ = '0' + num;
	}

#undef ITER
	
#endif

	// Add null terminator
	*out = 0;

	return ret;
}
