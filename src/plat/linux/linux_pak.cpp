// Linux pak file interface
// NOTE: Should I store the actual entry names to log better errors?

#include "types.h"
#include "pak.h"
#include "mem.h"
#include "str.h"
#include "util.h"
#include "endianUtil.h"

#include <string.h>

// Entry container, contains information for opening and closing pak entry,
// as well as pak entry itself
struct entry_container_t {
	void *mapAddr; // Current mapping address
	uptr mapSize; // Current mapping size
	uptr ref; // Number of instances of this pak file open

	uptr offset; // Offset into file
	uptr size; // Size in file

	int file; // File containing entry
	str_hash_t name; // String hash of entry name

	// Entry struct
	pak_entry_t entry;
};

// Get entry container from pak entry
static constexpr uptr entryContainerOffset = sizeof(entry_container_t)-sizeof(pak_entry_t);
FINLINE entry_container_t &entryParent(pak_entry_t *child) {
	return *(entry_container_t*)((u8*)child-entryContainerOffset);
}

void pak_entry_t::close() {
	entry_container_t &parent = entryParent(this);

	if (!parent.ref) {
		log_warning("Closing pak file %08x that's not open!", parent.name);
		return;
	}

	if (--parent.ref == 0) {
		munmap(parent.mapAddr, parent.mapSize);
		parent.mapAddr = NULL;
	}
}

// Internal data for the pak
struct pak_t::internal_t {
	uptr pageSize;

	uptr fileCount, maxFiles;
	uptr entryCount, maxEntries;

	int *files; // Buffer of file descriptors
	entry_container_t *entries; // Buffer of pak entries

	// Initialize pak
	FINLINE void init(mem_t &m, uptr initFiles, uptr initEntries) {
		pageSize = sysconf(_SC_PAGESIZE);
		fileCount = entryCount = 0;
		maxFiles = initFiles;
		maxEntries = initEntries;

		files = m.alloc(initFiles*sizeof(int));
		entries = m.alloc(initEntries*sizeof(entry_container_t));

		memset(entries, 0, initEntries*sizeof(entry_container_t));
	}

	// Resize pak memory pool
	FINLINE void resize(mem_t &m, uptr newMaxFiles, uptr newMaxEntries) {
		if (newMaxFiles > maxFiles) {
			int *newFiles = m.alloc(newMaxFiles*sizeof(int));
			memcpy(newFiles, files, sizeof(int)*fileCount);
			m.free(files);
			files = newFiles;
			maxFiles = newMaxFiles;
		}

		if (newMaxEntries > maxEntries) {
			entry_container_t *newEntries = m.alloc(newMaxEntries*sizeof(entry_container_t));
			memset(newEntries, 0, sizeof(entry_container_t)*newMaxEntries);
			memcpy(newEntries, entries, sizeof(entry_container_t)*entryCount);
			m.free(entries);
			entries = newEntries;
			maxEntries = newMaxEntries;
		}
	}
};

pak_t::pak_t(mem_t &m) : m_m(m) {
	m_d.init(m, 8, 256);
}

ubool pak_t::merge(const char *pak) {
	// Open pak file
	int fd = ::open(pak, O_RDONLY);
	if (fd < 0) return false;

	// Read pak file header
	pak_file_header_t hdr;
	read(fd, &hdr, sizeof(pak_file_header_t));

	// Verify this is a pak file
	if (hdr.magic != PAK_MAGIC) {
		close(fd);
		return false;
	}

	// Allocate and read entries
	uptr entryCount = hdr.entryCount;
	entries = (pak_file_entry_t*)m_m.alloc(sizeof(pak_file_entry_t*)*entryCount);
	read(fd, entries, sizeof(pak_file_entry_t*)*entryCount);

	// Merge entries into current entry table
	while (entryCount--) {
		if (m_d.entryCount >= m_d.maxEntries)
			m_d.resize(m_m, m_d.maxFiles, m_d.maxEntries*2);
		
		pak_file_entry_t &e = entries[entryCount];
		entry_container_t &o = m_d.entries[m_d.entryCount];

		o.offset = e.offset;
		o.size = e.size;
		o.hash = e.hash;
		o.file = fd;
		o.ref = 0;
		o.mapAddr = NULL;

		++m_d.entryCount;
	}

	// Merge successful, add file to fd list
	if (m_d.fileCount >= m_d.maxFiles)
		m_d.resize(m_m, m_d.maxFiles*2, m_d.maxEntries);
	
	m_d.files[m_d.fileCount++] = fd;

	return true;
}

pak_entry_t &pak_t::open(str_hash_t hash) {
	const entry_container_t * const end = m_d.entries + m_d.entryCount;

	for (entry_container_t *e = m_d.entries; e != end; ++e) {
		if (hash != e->hash) continue;

		// Entry found!

		// If entry is already open, add reference
		if (e->ref) ++e->ref;
		else {
			// Map entry portion of pak file to memory
			const off_t offset = util_alignDown<off_t>(e->offset, m_d.pageSize);
			e->mapSize = e->size + (e->offset-offset);
			e->map = mmap(NULL, e->ampSize, PROT_READ, MAP_PRIVATE, e->file, offset);

			// If mapping failed, return NULL
			if (e->mapAddr == MAP_FAILED) {
				log_warning("Mapping of %08x failed! (%d, %s)", errno, strerror(errno));
				
				e->mapAddr = NULL;
				return NULL;
			}

			// Return mapping
			e->entry.data = (u8*)e->mapAddr + (e->offset-offset);
			e->entry.size = e->size;
		}

		return e->entry;
	}

	// No entry found, return NULL
	log_warning("Entry %08x not found!", hash);
	return NULL;
}
