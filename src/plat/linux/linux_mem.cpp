#include "types.h"
#include "mem.h"
#include "log.h"

#include <sys/mman.h>

#include <cstring>
#include <cerrno>

// Must return value aligned to 16-byte boundary
void *mem_t::allocBlock(uptr size) {
	void *ret = mmap(NULL, size,
					 PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS,
					 -1, 0);

	if (ret == MAP_FAILED) {
		log_warning("Failed to allocate memory block! (%d, %s)\n",
					(int)errno, strerror(errno));
		return NULL;
	}

	// Make sure the address is aligned to a 16-byte boundary
	if ((uptr)ret&0xf) {
		log_warning("Memory block isn't aligned to a 16-byte boundary!");
		munmap(ret, size);
		return NULL;
	}

	return ret;
}

void mem_t::freeBlock() {
	munmap(m_start, size());
}
