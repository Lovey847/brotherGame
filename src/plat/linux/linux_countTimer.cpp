#define _POSIX_C_SOURCE 199309L // We need clock_gettime

#include "types.h"
#include "countTimer.h"

#include <ctime>

countTimer_counts_t countTimer_t::resolution() {return 1000000000;}
countTimer_counts_t countTimer_t::time() {
	struct timespec ret;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ret);

	return ret.tv_sec*1000000000 + ret.tv_nsec;
}

static constexpr countTimer_counts_t SLEEPMARGIN = 200000;
void countTimer_t::sleep(countTimer_counts_t counts) {
	// Sleep for the specified amount of time, minus some margin
	// since nanosleep isn't incredibly accurate
	struct timespec t;

	const countTimer_counts_t old = time();
	if (counts > SLEEPMARGIN) {
		t.tv_sec = counts/1000000000;
		t.tv_nsec = counts%1000000000 - SLEEPMARGIN;
		if (t.tv_nsec < 0) {
			--t.tv_sec;
			t.tv_nsec += 1000000000;
		}

		// Block for certain period of time
		nanosleep(&t, NULL);
	}

	// Loop until the time has passed (very accurate)
	while (time()-old < counts);
}
