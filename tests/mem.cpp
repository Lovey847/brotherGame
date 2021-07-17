#include "types.h"
#include "mem.h"

#include "test.h"

#include <cstdlib>
#include <cstring>

static constexpr const uptr MEM_SIZE = 1024;

static u8 mem[MEM_SIZE];
static const u8 zeroMem[MEM_SIZE] = {0};

// Copied from mem.h, for convenience
struct alignas(16) entry_t {
	entry_t *next, *prev;
	ubool active;
};

int main() {
	// Garble up mem
	for (uptr i = 0; i < MEM_SIZE; ++i)
		mem[i] = rand()&0xff;
	
	// Test default constructor and programmed constructor
	mem_t m;
	TEST_FALSE(m.active());
	
	m = mem_t(mem, MEM_SIZE);
	TEST_TRUE(m.active());
	
	// Memory should be cleared
	TEST_MEM(mem, zeroMem, MEM_SIZE);
	
	// Test allocation
	u8 *addr = (u8*)m.alloc(64);
	TEST_EQUALS(addr, mem + sizeof(entry_t));
	
	// Get entry and next entry
	entry_t *ent = (entry_t*)(addr-sizeof(entry_t));
	entry_t *next = (entry_t*)(addr+64);
	
	TEST_EQUALS((u8*)ent, mem);
	TEST_EQUALS(ent->next, next);
	TEST_EQUALS(ent->prev, NULL);
	TEST_TRUE(ent->active);
	
	TEST_EQUALS((u8*)next, mem+64+sizeof(entry_t));
	TEST_EQUALS(next->next, NULL);
	TEST_EQUALS(next->prev, ent);
	TEST_FALSE(next->active);
	
	// Test one more allocation
	u8 *addr2 = (u8*)m.alloc(256);
	TEST_EQUALS(addr, mem + sizeof(entry_t)*2 + 64);
	
	entry_t *next2 = (entry_t*)(addr2+256);
	
	TEST_EQUALS(next2, next->next);
	TEST_EQUALS(next2->next, NULL);
	TEST_EQUALS(next2->prev, next);
	TEST_FALSE(next2->active);
	
	// Test frees
	m.free(addr);
	
	TEST_EQUALS(ent->next, next);
	TEST_EQUALS(ent->prev, NULL);
	TEST_FALSE(ent->active);
	
	TEST_EQUALS(next->next, next2);
	TEST_EQUALS(next->prev, ent);
	TEST_TRUE(next->active);
	
	// This free should recognize that a free section comes after
	// another free section, and remove the second free section
	m.free(addr2);
	
	TEST_EQUALS(ent->next, NULL);
	TEST_EQUALS(ent->prev, NULL);
	TEST_FALSE(ent->active);
	
	return 0;
}
