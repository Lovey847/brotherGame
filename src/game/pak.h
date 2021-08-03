#ifndef GAME_PAK_H
#define GAME_PAK_H

#include "types.h"
#include "file.h"
#include "endianUtil.h"
#include "mem.h"
#include "str.h"

/////////////////////
// Pak file format

// Pak file header
static constexpr u32 PAK_MAGIC = util_magic('P', 'A', 'K', '2');
struct pak_file_hdr_t {
  u32 magic;

  endian_u32 entryCount;

  u8 pad[4]; // Padding for 16-byte alignment
};

// Pak file entry
struct pak_file_entry_t {
  char name[64-20]; // Filename
  str_hash_t nameHash; // Filename hash

  endian_u32 offset; // File offset
  endian_u32 size; // File size
};
static_assert(sizeof(pak_file_entry_t) == 64, "");

/////////////////////////////////
// Pak file handling utilities

// Pointer in pak, contains relative address
template <typename T>
struct pak_ptr_t {
  endian_u32 ptr;

  FINLINE operator T*() {return (T*)((u8*)this+ptr);}
  FINLINE operator const T*() const {return (const T*)((u8*)this+ptr);}
  FINLINE T *operator->() {return (T*)((u8*)this+ptr);}
  FINLINE T *operator[](int i) {return *((T*)this + i);}

  // Implicit overload for void*
  FINLINE operator void*() {return (void*)((u8*)this+ptr);}
  FINLINE operator const void*() const {return (const void*)((u8*)this+ptr);}

  // Set pak pointer
  FINLINE void set(void *addr) {
    ptr = (u8*)addr-(u8*)this;
  }
};

// Pak entry ID
typedef uptr pak_entry_t;
static constexpr pak_entry_t PAK_INVALID_ENTRY = 0xffffffffu;

// Pak directory, contains mappable files
class pak_t {
private:
  // Pak entry
  struct entry_t;

  mem_t &m_m;
  file_system_t &m_f;

  file_handle_t *m_pak; // Pak file handle

  entry_t *entries; // Entry list
  uptr entryCount;

public:
  pak_t(mem_t &m, file_system_t &f, const char *filename);
  ~pak_t();

  // Get pak entry from string hash
  // Returns PAK_INVALID_ENTRY if no pak entry was found
  pak_entry_t getEntry(str_hash_t name);

  // Map pak entry
  // NULL is returned on error
  const void *mapEntry(pak_entry_t ent);

  // Unmap pak entry (invalidates mapped pointer)
  void unmapEntry(pak_entry_t ent);
};

#endif //GAME_PAK_H
