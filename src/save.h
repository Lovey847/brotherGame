// Save data management
// NOTE: Save data is serialized, so it must be endian-independent

#ifndef SAVE_H
#define SAVE_H

#include "types.h"
#include "str.h"
#include "endianUtil.h"
#include "util.h"
#include "log.h"
#include "mem.h"
#include "file.h"
#include "stack.h"

//////////////////////////////////
// BEGIN SAVE DATA FILE FORMATS
//////////////////////////////////

// Save data magic, aligns the rest of file to a 16-byte boundary
// Every save data version starts with this
struct alignas(16) save_data_magic_t {
	u32 magic;

	u32 pad[3];
};

// The magic isn't just a guard against invalid save data,
// it's also a descriptor of the save data version

////////////////////////////////
// Save data format, version 1
//
// Format:
//   save_data_magic_t magic; // magic == SAVE_DATA1_MAGIC
//   save_data1_var_t vars[]; // 0-size terminated

static constexpr u32 SAVE_DATA1_MAGIC = util_magic('S', 'V', '0', '1');

// Save file variable
// Data and variables are always aligned to a 16-byte boundary
struct alignas(16) save_data1_var_t {
	// Size of variable value
	// Aligned to 16-byte boundary
	endian_u32 size;
	
	str_hash_t name;

	// 1 padding field, for the variable data's alignment
	u32 pad[1];

	FINLINE ubool end() const {
		log_assert((size&0xf) == 0, "Invalid variable size %u!", size);
		return size != 0;
	}
};

///////////////////////////////
// END SAVE DATA FILE FORMATS
///////////////////////////////

// Save variable, in memory
struct save_var_t {
	void *val;
	uptr size;
	
	str_hash_t name;
};

class save_t {
private:
	mem_t &m_m;
	file_system_t &m_f;
	
	// Memory to store variable data (every variable is aligned to a 16-byte boundary)
	u8 *m_varMem, *m_varMemCur, *m_varMemEnd;

	// Variable buffers
	stack_t<save_var_t> m_gvars; // Global save variables: These are present until destruction
	stack_t<save_var_t> m_vars; // Save variables: These are loaded from a save slot

	// Currently loaded save file
	i32 m_curSave;

	// Save vars to file
	ubool saveFile(const char *filename, stack_t<save_var_t> &buf) const;

	// Load vars from file
	ubool loadFile(const char *filename, stack_t<save_var_t> &buf);

	// Add variable memory
	void *pushVarMem(const u8 *ptr, uptr size, uptr realSize);
	FINLINE void *pushVarMem(const u8 *ptr, uptr size) {
		return pushVarMem(ptr, size, util_alignUp<uptr>(size, 16));
	}
	
	// Clean variable memory (get rid of loaded save file variables)
	void cleanVarMem();

public:
	save_t(mem_t &m, file_system_t &f);
	~save_t();

	// Get value from save data
	//
	// If global is true, variable is taken from the global save file,
	// otherwise, variable is taken from the loaded save file
	//
	// If outSize is smaller than size of value, then value is truncated to outSize
	//
	// Returns size of value
	// Returns 0 if the value doesn't exist
	uptr get(str_hash_t name, void *out, uptr outSize, ubool global) const;

	// Set value in save data
	//
	// If global is true, variable is written to the global save file,
	// otherwise, variable is written to the loaded save file
	//
	// Returns false if inSize is greater than the value's size,
	// or if memory ran out while creating a new variable
	// If value doesn't exist, the value is created with at least inSize bytes
	ubool set(str_hash_t name, const void *in, uptr inSize, ubool global);

	// Templated overloads
	template<typename T>
	FINLINE uptr get(str_hash_t name, T &out, ubool global) const {
		return get(name, (void*)out, sizeof(T), global);
	}

	template<typename T>
	FINLINE ubool set(str_hash_t name, const T &in, ubool global) {
		return set(name, (void*)in, sizeof(T), global);
	}

	template<typename T>
	FINLINE uptr get(str_hash_t name, T *out, uptr outCount, ubool global) const {
		return get(name, (void*)out, sizeof(T) * outCount, global);
	}

	template<typename T>
	FINLINE ubool set(str_hash_t name, const T *in, uptr inCount, ubool global) {
		return set(name, (void*)in, sizeof(T) * inCount, global);
	}

	// Check if variable is present
	FINLINE ubool check(str_hash_t name, ubool global) const {
		return get(name, NULL, 1, global);
	}

	// Make variable present in file
	FINLINE ubool set(str_hash_t name, ubool global) {
		return set(name, NULL, 1, global);
	}

	// Load vars from save file
	// Unloads previous save file, can only load one save file at a time, apart from the global save file
	ubool load(u32 id);
};

#endif //SAVE_H
