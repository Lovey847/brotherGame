#include "types.h"
#include "log.h"
#include "util.h"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <ctime>

void log_note_manual(const char *file, u32 line, const char *str, ...) {
	va_list args;
	va_start(args, str);
	
	printf("%s, %u: ", file, line);
	vprintf(str, args);
	putchar('\n');
	
	va_end(args);
}

void log_warning_manual(const char *file, u32 line, const char *str, ...) {
	va_list args;
	va_start(args, str);
	
	printf("!! %s, %u !!: ", file, line);
	vprintf(str, args);
	putchar('\n');
	
	va_end(args);
}

log_except_t::log_except_t(const char *file, u32 line, const char *s, ...) {
	va_list args;
	va_start(args, s);

	vsnprintf(m_err, sizeof(m_err), s, args);
	snprintf(m_err+strlen(m_err), sizeof(m_err)-strlen(m_err), " (%s, %u)", file, line);

	va_end(args);
}
