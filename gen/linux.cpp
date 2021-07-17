/*
 * Linux entry point for generators
 */

#include "types.h"

#include "gen.h"
#include "mem.h"
#include "file.h"
#include "linux_file.h"

#include <cstdio>

int main(int argc, char **argv) {
	// First argument should be text filename
	if (argc <= 2) {
		printf("Usage: %s <generator txt> <output directory>\n",
			   argv[0]);
		return 1; // Return failure, this shouldn't happen in the build step
	}

	// Initialize memory pool
	mem_t mem(8*1024*1024);

	// Initialize file system
	linux_file_system_t sys(mem);

	try {
		gen_main(mem, *(file_system_t*)&sys, argv[1], argv[2]);
		return 0;
	} catch (const log_except_t &err) {
		log_warning("Generator failed: %s", err.str());
		return 1;
	}
}
