#include "types.h"
#include "mem.h"
#include "log.h"
#include "util.h"

#include <cstring>

#define LOG_ALLOC 0

mem_t::mem_t(uptr size) {
	log_assert((size&0xf) == 0, "Misaligned size!");
	
	m_start = allocBlock(size);
	if (!m_start) throw log_except("Failed to allocate memory block!");
	
	m_end = (void*)((u8*)m_start + size);
	
	// Clear memory
	memset(m_start, 0, size);
}

mem_t::~mem_t() {
	uptr bytes = 0;
	
	// Perform quick check on memory
	entry_t *i = (entry_t*)m_start;
//	for (entry_t *i = (entry_t*)m_start; i->next; i = i->next) {
	for (;;) {
		if (!i->next) break;
		else if ((i->next < (entry_t*)m_start) ||
				 (i->next >= (entry_t*)m_end))
		{
			log_warning("Memory arena corrupted at %016llx!", (unsigned long long)i);
			break;
		}
		
		if (i->active) bytes += i->size();

		i = i->next;
	}

	if (bytes) log_warning("Leaked %u bytes of memory!", (u32)bytes);

	freeBlock();
}

void *mem_t::allocTemp(uptr size) const {
	// Start at beginning of memory and find an unused entry
	// with enough size
	entry_t *i;
	for (i = (entry_t*)m_start; i->next; i = i->next) {
		if (i->active) continue;
		if (i->size() >= size) break;
	}
	
	// If the last entry is active, or the entry isn't big enough, we're out of memory
	if (i->active || (i->size() < size))
		throw log_except("Cannot allocate %u bytes of temporary memory!",
						 (u32)size);

	// Return temporary data
	return i->data();
}

void *mem_t::alloc(uptr size) {
	log_assert(size != 0, "Allocating nothing!");
	
	// Align size up to 16-byte boundary
	size = util_alignUp<uptr>(size, 16);
	
	// Start at beginning of memory and find an unused entry
	// with enough size
	entry_t *i;
	for (i = (entry_t*)m_start; i->next; i = i->next) {
		if (i->active) continue;
		if (i->size() >= size) break;
	}
	
	// If the last entry is active, we're out of memory
	if (i->active) throw log_except("Cannot allocate %u bytes of memory!",
									(u32)size);
	
	entry_t * const next = (entry_t*)((u8*)i->data() + size);
	
	// If next pointer is past the end-point, we're out of memory
	if (next->data() > m_end)
		throw log_except("Cannot allocate %u bytes of memory!",
						 (u32)size);
	
	// If the new next entry would be inside the next entry, don't make it
	// Also don't make entries with a size of 0
	if (!i->next || ((uptr)((u8*)i->next - (u8*)next) > sizeof(entry_t))) {
		// Fit next into list
		next->next = i->next;
		next->prev = i;
		next->active = false;
		
		i->next = next;
	}

#if LOG_ALLOC
	log_note("Allocating %u bytes of memory at %016llx", (u32)size, (unsigned long long)i->data());
#endif

	i->active = true;
	return i->data();
}

void mem_t::free(void *addr) {
	// Get entry from address
	entry_t *ent = (entry_t*)addr-1;
	
	log_assert(ent->active, "Invalid free of address 0x%016llx!", (unsigned long long)addr);

#if LOG_ALLOC
	log_note("Freed address of size %u at %016llx", (u32)ent->size(), (unsigned long long)addr);
#endif
	
	// If the previous entry is free, do processing from there
	// Since there should never be 2 free entries in a row, only check 1 entry behind this one
	if (ent->prev && !ent->prev->active) {
		// Since the previous entry is free, we must act like this entry doesn't exist
		ent->prev->next = ent->next;
		ent = ent->prev;
	}
	
	// If the next entry is free, act as if it doesn't exist.
	if (ent->next && !ent->next->active)
		ent->next = ent->next->next;
	
	// Mark the entry as free
	ent->active = false;
}
