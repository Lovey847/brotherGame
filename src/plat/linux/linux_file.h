// Derived platform-specific classes for linux

#ifndef LINUX_FILE_H
#define LINUX_FILE_H

#include "types.h"
#include "mem.h"
#include "file.h"
#include "buffer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>

// Forward declarations
class linux_file_mapping_t;
class linux_file_handle_t;
struct linux_file_res_t;
class linux_file_system_t;

class linux_file_mapping_t : public file_mapping_t {
public:
	linux_file_res_t *m_res;
	
	void *m_addr; // Real mapping address, aligned to page size, used in unmap
	uptr m_size; // Real mapping size, also used in unmap

	// Methods
	FINLINE linux_file_mapping_t() {
		data = NULL;
	}
	~linux_file_mapping_t();
	
	linux_file_mapping_t(const linux_file_mapping_t &other) = delete;
	
	// Called by linux_file_handle_t to map portion of file
	ubool map(uptr offset, uptr sz, uptr realOffset, uptr realSize, int fd, file_mapmode_t mode, linux_file_res_t &res);

	// void unmap();
};

class linux_file_handle_t : public file_handle_t {
public:
	linux_file_res_t *m_res;
	
	int m_fd; // Linux file handle

	file_mode_t m_mode;

	// Methods
	FINLINE linux_file_handle_t() : m_res(NULL) {}
	~linux_file_handle_t();

	linux_file_handle_t(const linux_file_handle_t &other) = delete;

	// Called by linux_file_system_t to open the file after default construction
	ubool open(linux_file_res_t &res, const char *filename, file_mode_t mode);

	// void close();
	// iptr read(void *out, uptr bytes);
	// iptr write(const void *in, uptr bytes);
	// ubool seek(uptr offset, file_seek_t orig);
	// iptr tell();
	// file_mapping_t *map(file_mapmode_t mode, uptr offset, uptr size);
};

// Linux file system resources
struct linux_file_res_t {
	mem_t &m;
	uptr pageMask;

	// So I originally made this with mappings before handles,
	// and that's why in almost everything here the mappings come before the handles,
	// but I realized if I do that then I'm closing the handles before the mapping,
	// and that could cause some issues.
	buffer_t<linux_file_handle_t> handles;
	buffer_t<linux_file_mapping_t> mappings;

	FINLINE linux_file_res_t(mem_t &mem, uptr maxMappings, uptr maxHandles) :
		m(mem), pageMask(sysconf(_SC_PAGESIZE)), handles(m, maxHandles), mappings(m, maxMappings)
	{
		if ((pageMask - 1)&pageMask) throw log_except("Page size isn't a power of 2! (%u)", (u32)pageMask);

		// I want a mask to bitwise-and offsets with
		--pageMask;
	}

	linux_file_res_t(const linux_file_res_t &other) = delete;
};

// Linux file system
class linux_file_system_t : public file_system_t {
public:
	mem_t &m;
	
	linux_file_res_t m_res;

	// Methods
	linux_file_system_t(mem_t &mem);

	linux_file_system_t(const linux_file_system_t &other) = delete;

	// file_handle_t *open(const char *filename, file_mode_t mode);
	// ubool fileExists(const char *filename);
	// ubool makeDir(const char *dirname);
	// ubool dirExists(const char *dirname);
};

#endif //LINUX_FILE_H
