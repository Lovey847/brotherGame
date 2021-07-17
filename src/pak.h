// Contains an interface to the pak, which can import pak files

#ifndef PAK_H
#define PAK_H

#include "types.h"
#include "str.h"
#include "util.h"
#include "endianUtil.h"
#include "mem.h"
#include "file.h"

// Pointer in pak file, relative to itself
template<typename T>
struct pak_ptr_t {
	endian_u32 offset;

	FINLINE T *ptr() {
		return (T*)((u8*)base + offset);
	}
};

// Pak file format

// Pak file header
static constexpr u32 PAK_MAGIC = util_magic('P', 'A', 'K', '2');
struct pak_file_header_t {
	u32 magic;
	endian_u32 entryCount;

	u8 pad[4];
};

// Pak file entry
struct pak_file_entry_t {
	char name[64 - 20]; // Entry name, used by pak editors
	str_hash_t hash; // Entry name hash, used by the game;

	endian_u32 offset; // Offset of entry in pak file
	endian_u32 size; // Size of entry in pak file
};
static_assert(sizeof(pak_file_entry_t) == 64, "");

// Pak entry, pointer returned by pak_t::open
struct pak_entry_t {
	const void *data;
	uptr size;

	void close(); // Close entry
};

// The pak, contains entries for all pak files merged
class pak_t {
private:
	mem_t &m_m;
	
	// Internal data
	struct internal_t;
	mem_static_container_t<internal_t, 64, 16> m_d;

public:
	pak_t(mem_t &m);
	~pak_t();

	// Returns false if merging failed
	ubool merge(const char *pak);

	// Open pointer to pak entry data
	// Returns NULL if not found
	pak_entry_t *open(str_hash_t hash);
};

#endif //PAK_H
