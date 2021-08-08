#include "types.h"
#include "mem.h"
#include "file.h"
#include "linux_file.h"
#include "log.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

linux_file_mapping_t::~linux_file_mapping_t() {
	linux_file_mapping_t &me = *(linux_file_mapping_t*)this;
	
	munmap(me.m_addr, me.m_size);
	size = (uptr)(data = 0);
}

ubool linux_file_mapping_t::map(uptr offset, uptr sz, uptr realOffset, uptr realSize,
                                int fd, file_mapmode_t mode, linux_file_res_t &res)
{
  // Get resources
  m_res = &res;

	// Determine permissions
	int prot, flags;

	switch (mode) {
	case FILE_MAP_READ:
		prot = PROT_READ; flags = MAP_PRIVATE; break;
	case FILE_MAP_READWRITE:
		prot = PROT_READ|PROT_WRITE; flags = MAP_SHARED; break;
	}

	// Map file to memory
	m_size = realSize;

	m_addr = mmap(NULL, realSize, prot, flags, fd, realOffset);
	if (m_addr == MAP_FAILED) {
		log_warning("Cannot map file at offset %u with size %u to memory! (%d, %s)",
					(u32)realOffset, (u32)realSize, errno, strerror(errno));
		return false;
	}

	// Mapping successful: set public variables
	data = (void*)((u8*)m_addr + offset-realOffset);
	size = sz;

	return true;
}

void file_mapping_t::unmap() {
	linux_file_mapping_t &me = *(linux_file_mapping_t*)this;

	me.m_res->mappings.remove(me);
}

linux_file_handle_t::~linux_file_handle_t() {
	// I'd like to check for mappings here, but there's not really a way to do that
	::close(m_fd);
}

ubool linux_file_handle_t::open(linux_file_res_t &res, const char *filename, file_mode_t mode) {
	log_assert(!m_res, "File is already opened!");
	
	m_mode = mode;
	
	// Determine permissions
	switch (mode) {
	case FILE_MODE_READ: m_fd = ::open(filename, O_RDONLY); break;
	case FILE_MODE_WRITE: m_fd = ::open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0664); break;
	case FILE_MODE_READWRITE: m_fd = ::open(filename, O_RDWR|O_CREAT, 0664); break;
	case FILE_MODE_APPEND: m_fd = ::open(filename, O_WRONLY|O_CREAT|O_APPEND, 0664); break;
	}

	if (m_fd < 0) {
		log_warning("Cannot open %s! (%d, %s)", filename, errno, strerror(errno));
		return false;
	}

	m_res = &res;

	return true;
}

void file_handle_t::close() {
	linux_file_handle_t &me = *(linux_file_handle_t*)this;

	me.m_res->handles.remove(me);
	me.m_res = NULL;
}

iptr file_handle_t::read(void *out, uptr bytes) {
	linux_file_handle_t &me = *(linux_file_handle_t*)this;
	
	if (!me.m_res) {
		log_warning("Attempting to read bytes from unopened file!");
		return -1;
	}

	// Check if the mode is readable, very convenient enum order I must say
	if (me.m_mode&1) {
		log_warning("File doesn't support reading! (%d)", (int)me.m_mode);
		return false;
	}

	ssize_t ret = ::read(me.m_fd, out, bytes);
	if (ret == -1)
		log_warning("Cannot read %u bytes from file! (%d, %s)", (u32)bytes, errno, strerror(errno));
	
	return ret;
}

iptr file_handle_t::write(const void *in, uptr bytes) {
	linux_file_handle_t &me = *(linux_file_handle_t*)this;
	
	if (!me.m_res) {
		log_warning("Attempting to write bytes to unopened file!");
		return -1;
	}

	// Check if the mode is writeable
	if (me.m_mode == FILE_MODE_READ) {
		log_warning("File doesn't support writing! (%d)", (int)me.m_mode);
		return false;
	}

	ssize_t ret = ::write(me.m_fd, in, bytes);
	if (ret == -1)
		log_warning("Cannot write %u bytes to file! (%d, %s)", (u32)bytes, errno, strerror(errno));

	return ret;
}

ubool file_handle_t::seek(uptr offset, file_seek_t orig) {
	// Lookup table for file_seek_t
	const int whenceTable[] = {
		SEEK_SET, // FILE_SEEK_SET
		SEEK_CUR, // FILE_SEEK_REL
		SEEK_END // FILE_SEEK_END
	};

	linux_file_handle_t &me = *(linux_file_handle_t*)this;

	if (lseek(me.m_fd, offset, whenceTable[orig]) < 0) {
		log_warning("Cannot seek to offset %u from origin %d in file! (%d, %s)",
					(u32)offset, (int)orig, errno, strerror(errno));
		return false;
	}

	return true;
}

iptr file_handle_t::tell() {
	linux_file_handle_t &me = *(linux_file_handle_t*)this;
	
	off_t ret = lseek(me.m_fd, 0, SEEK_CUR);
	if (ret < 0)
		log_warning("Cannot give position of cursor in file! (%d, %s)",
					errno, strerror(errno));

	return ret;
}

file_mapping_t *file_handle_t::map(file_mapmode_t mode, uptr offset, uptr size) {
	linux_file_handle_t &me = *(linux_file_handle_t*)this;
	
	uptr realOffset, realSize;

	realOffset = offset & ~me.m_res->pageMask;
	realSize = size + (offset&me.m_res->pageMask);

	linux_file_mapping_t *ret = &me.m_res->mappings.add();
	if (!ret->map(offset, size, realOffset, realSize, me.m_fd, mode, *me.m_res)) {
		log_warning("Cannot map file from %u to %u!", (u32)offset, (u32)offset+size);

		me.m_res->mappings.remove(*ret);
		return NULL;
	}

	return (file_mapping_t*)ret;
}

linux_file_system_t::linux_file_system_t(mem_t &mem) : m(mem), m_res(m, 256, 64) {}

file_handle_t *file_system_t::open(const char *filename, file_mode_t mode) {
	linux_file_system_t &me = *(linux_file_system_t*)this;

	linux_file_handle_t *ret = &me.m_res.handles.add();
	if (!ret->open(me.m_res, filename, mode)) {
		log_warning("Cannot open file %s in mode %d!", filename, (int)mode);

		me.m_res.handles.remove(*ret);
		return NULL;
	}

	return ret;
}

ubool file_system_t::fileExists(const char *filename) {
	if (access(filename, F_OK) >= 0) return true;

	if (errno != ENOENT) {
		log_warning("Unexpected error from access for %s! (%d, %s)", filename, errno, strerror(errno));
		return true;
	}

	return false;
}

ubool file_system_t::makeDir(const char *dirname) {
	if (mkdir(dirname, 0664) < 0) {
		if ((errno == EEXIST) && !fileExists(dirname)) return true;

		log_warning("Cannot create directory %s! (%d, %s)", dirname, errno, strerror(errno));
		return false;
	}

	return true;
}

ubool file_system_t::dirExists(const char *dirname) {
	struct stat s;
	
	if (stat(dirname, &s) < 0) {
		if (errno != ENOENT) {
			log_warning("Unexpected error from stat for %s! (%d, %s)", dirname, errno, strerror(errno));
			return true;
		}

		return false;
	}
	
	return (s.st_mode&S_IFDIR) != 0;
}
