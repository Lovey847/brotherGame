#ifndef LOG_H
#define LOG_H

#include "types.h"

#include <cassert>

// TODO: Give log_note and log_warning a file argument
#define log_note(...) log_note_manual(__FILE__, __LINE__, __VA_ARGS__)
#define log_warning(...) log_warning_manual(__FILE__, __LINE__, __VA_ARGS__)
#define log_assert(condition, ...) assert(condition) // TODO: Get rid of log_assert
#define log_except(...) log_except_t(__FILE__, __LINE__, __VA_ARGS__)

void log_note_manual(const char *file, u32 line, const char *str, ...);
void log_warning_manual(const char *file, u32 line, const char *str, ...);

// Log exception class, for throwing exceptions with descriptive explanations
// Example usage: throw log_except("string", value1, value2, ...)
class log_except_t {
private:
	char m_err[1024];

public:
	log_except_t(const char *s, ...);
	log_except_t(const char *file, u32 line, const char *s, ...);

	FINLINE const char *str() const {return m_err;}
};

#endif //LOG_H
