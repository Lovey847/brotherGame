// NOTE: Don't call anything file_t, apparently the same name is used in a linux header and
//       it messes things up on linux!

#ifndef FILE_H
#define FILE_H

#include "types.h"
#include "mem.h"

// File open mode enum
enum file_mode_t {
	FILE_MODE_READ = 0,
	FILE_MODE_WRITE,
	FILE_MODE_READWRITE,

	// Write only, but instead of erasing the contents of the file, it sets
	// the cursor to the end of the file
	FILE_MODE_APPEND
};

// File seek mode enum
enum file_seek_t {
	FILE_SEEK_SET = 0, // Beginning of file
	FILE_SEEK_REL, // Relative to cursor
	FILE_SEEK_END // Backwards from end of file
};

// File mapping mode enum
enum file_mapmode_t {
	FILE_MAP_READ = 0,
	FILE_MAP_READWRITE
};

// File mapping
class file_mapping_t {
public:
	// Unmap portion of file from memory
	// After unmapping, data and size will be zeroed out
	void unmap();

	// Fast access to memory store

	// Pointer to portion of file
	// Writing is disallowed if read-only, reading is disallowed if write-only
	void *data;

	// Size of portion of file
	uptr size;
};

// File handle
class file_handle_t {
public:
	// Close the file handle
	// THE FILE SHOULD NOT BE REFERENCED AT ALL AFTER CLOSING
	void close();

	// Read from file handle
	// Returns number of bytes read
	// Returns -1 on error
	iptr read(void *out, uptr bytes);

	// Write to file handle
	// Returns number of bytes written
	// Returns -1 on error
	iptr write(const void *in, uptr bytes);

	// Set cursor position in file
	ubool seek(uptr offset, file_seek_t orig);

	// Returns absolute cursor position in file
	// Returns -1 on error
	iptr tell();

	// Map portion of file to memory
	// Note that only files opened in FILE_MODE_READ or FILE_MODE_READWRITE mode may be mapped to memory
	// Files opened with FILE_MODE_READ must be opened in the FILE_MAP_READ mode
	// NULL is returned on error
	file_mapping_t *map(file_mapmode_t mode, uptr offset, uptr size);
};

// File module system
class file_system_t {
public:
	// Open new file handle
	// NULL is returned on error
	file_handle_t *open(const char *filename, file_mode_t mode);

	// Check if file exists
	ubool fileExists(const char *fiename);

	// Create directory
	// If directory already exists, nothing happens
	// Return false on error
	ubool makeDir(const char *dirname);

	// Check if directory exists
	ubool dirExists(const char *dirname);
};

#endif //FILE_H
