#ifndef COUNTTIMER_H
#define COUNTTIMER_H

#include "types.h"

// Timer counts type
typedef u64 countTimer_counts_t;

// Interface class for timer, using "counts"
// A count can be any unit of time
class countTimer_t {
public:
	// Various functions for getting the current time
	// When the timer started is undefined

	// Get number of counts in a second
	countTimer_counts_t resolution();

	// Get current counts
	countTimer_counts_t time();

	// Sleep for counts
	void sleep(countTimer_counts_t counts);
};

#endif //COUNTTIMER_H
