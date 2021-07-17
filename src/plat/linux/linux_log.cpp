#include "log.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>

void log_warning(const char *str, ...) {
	// Append to error log
	// TODO: Implement an error message box at some point!
	
	int fd = open("err.log", O_WRONLY|O_CREAT|O_APPEND);

	// Can't exactly log an error here...
	if (fd < 0) return;

	va_list args;
	va_start(args, str);

	char s[1024];
	vsnprintf(s, sizeof(s), str, args);

	va_end(args);

	write(fd, "!! ", sizeof("!! "));
	write(fd, s, strlen(s));
	write(fd, " !!\n", sizeof(" !!\n"));

	close(fd);
}

[[noreturn]] void log_error(const char *str, ...) {
	// Append to error log and quit
	// TODO: Implement an error message box at some point!

	int fd = open("err.log", O_WRONLY|O_CREAT|O_APPEND);

	// Can't exactly log an error here...
	if (fd < 0) exit(1);

	va_list args;
	va_start(args, str);

	char s[1024];
	vsnprintf(s, sizeof(s), str, args);

	va_end(args);

	write(fd, "!! ", sizeof("!! "));
	write(fd, s, strlen(s));
	write(fd, " !!\n", sizeof(" !!\n"));

	close(fd);

	exit(1);
}
