#include "types.h"
#include "pak.h"
#include "file.h"
#include "endianUtil.h"
#include "mem.h"
#include "log.h"

// Entry memory layout
struct pak_t::entry_t {
  // File offset and size
  uptr offset, size;

  // Entry mapping, NULL if unmapped
  file_mapping_t *mapping;

  // Entry references
  uptr ref;

  // Only read name hash
  str_hash_t nameHash;
};

pak_t::pak_t(mem_t &m, file_system_t &f, const char *filename) :
  m_m(m), m_f(f)
{
  // Open pak file
  m_pak = m_f.open(filename, FILE_MODE_READ);
  if (!m_pak) throw log_except("Cannot open %s!", filename);

  // Read pak file header
  pak_file_hdr_t hdr;
  m_pak->read(&hdr, sizeof(hdr));

  // Some sanitizing
  if (hdr.magic != PAK_MAGIC) throw log_except("Invalid pak file magic!");
  if (hdr.entryCount == 0) throw log_except("No entries in pak!");

  // Valid pak file header, allocate entries
  entryCount = hdr.entryCount;
  entries = (entry_t*)m_m.alloc(sizeof(entry_t)*entryCount);

  // Read entries into memory
  for (entry_t *e = entries+entryCount; e-- != entries;) {
    pak_file_entry_t ent;
    m_pak->read(&ent, sizeof(pak_file_entry_t));

    e->offset = ent.offset;
    e->size = ent.size;
    e->mapping = NULL;
    e->ref = 0;
    e->nameHash = ent.nameHash;
  }

  // Pak file has been read into memory
}

pak_t::~pak_t() {
  // Go through entry list, unmapping mapped entries
  for (entry_t *e = entries+entryCount; e-- != entries;) {
    if (e->mapping) {
      log_warning("Entry still mapped at exit!");
      e->mapping->unmap();
    }
  }

  // Close pak file and free entries
  m_pak->close();
  m_m.free(entries);
}

pak_entry_t pak_t::getEntry(str_hash_t name) {
  // Go through entries, searching for name
  for (entry_t *e = entries+entryCount; e-- != entries;)
    if (e->nameHash == name) return e-entries;

  return PAK_INVALID_ENTRY;
}

// Map entry to memory
const void *pak_t::mapEntry(pak_entry_t ent) {
  if (ent >= entryCount) return NULL;

  // If entry isn't mapped, map entry
  if (!entries[ent].mapping) {
    entries[ent].mapping = m_pak->map(FILE_MAP_READ, entries[ent].offset, entries[ent].size);
    if (!entries[ent].mapping) return NULL;
  } else {
    // Otherwise, increment ref count
    ++entries[ent].ref;
  }

  return entries[ent].mapping->data;
}

// Unmap entry from memory
void pak_t::unmapEntry(pak_entry_t ent) {
  // If entry isn't mapped, give warning
  if (!entries[ent].mapping) {
    log_warning("Unmapping entry that isn't mapped!");
    return;
  }

  // If ref count is greater than 0, decrement ref count
  if (entries[ent].ref) --entries[ent].ref;
  else {
    // Otherwise, unmap entry
    entries[ent].mapping->unmap();
    entries[ent].mapping = NULL;
  }
}
