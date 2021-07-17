// PAK generator

#include "gen.h"
#include "str.h"
#include "util.h"
#include "endianUtil.h"
#include "file.h"
#include "log.h"

#include <cstring>

static file_system_t *sys;
static mem_t *mem;

struct hdr_t {
	u32 magic;

	endian_u32 entryCount;

	u8 pad[4]; // padding for 16-byte alignment
};

struct entry_t {
	char name[64 - 20];
	str_hash_t hash;

	endian_u32 offset;
	endian_u32 size;
};
static_assert(sizeof(entry_t) == 64, "");

// File buffers
#define MAXFILES 256
static file_handle_t *files[MAXFILES];
static entry_t fileEntries[MAXFILES];
static uptr fileCount = 0;
static u32 curOffset;

// Filename buffer
#define FILENAMESLEN 4096
static char filenames[FILENAMESLEN], *curFilename = filenames;
#define endFilenames (filenames+FILENAMESLEN)

// Functions
static ubool addFile(const char *fname) {
	// Error out if out of memory
	if (fileCount >= MAXFILES) {
		log_warning("Too many files! (increase MAXFILES in gen/data/gen.cpp)");
		return false;
	}

	file_handle_t *f = sys->open(fname, FILE_MODE_READ);
	if (!f) return false;

	f->seek(0, FILE_SEEK_END);
	uptr fileSize = f->tell();
	f->seek(0, FILE_SEEK_SET);

	files[fileCount] = f;

	entry_t *e = fileEntries + fileCount;

	// Omit "files/"
	memcpy(e->name, fname+6, strlen(fname+6));
	e->hash = str_hashR(e->name);
	e->offset = curOffset;
	e->size = fileSize;

	// Align all data to a 16-byte boundary
	curOffset += util_alignUp<u32>(e->size, 16);
	++fileCount;

	return true;
}

static ubool writeFile(file_handle_t *f, file_handle_t *pak, entry_t *e) {
	u8 buf[2048];

	uptr fileSize = util_alignUp<u32>(e->size, 16);
	while (fileSize) {
		const uptr size = util_min<uptr>(fileSize, sizeof(buf));
		f->read(buf, size);
		pak->write(buf, size);

		fileSize -= size;
	}

	return true;
}

static ubool writePak(const char *fname) {
	file_handle_t **f;
	entry_t *e;

	file_handle_t *pak;

	hdr_t hdr;

	pak = sys->open(fname, FILE_MODE_WRITE);
	if (!pak) return false;

	// Current pak version
	hdr.magic = util_magic('P', 'A', 'K', '2');
	hdr.entryCount = fileCount;
	pak->write(&hdr, sizeof(hdr_t));

	pak->write(fileEntries, sizeof(entry_t)*fileCount);

	for (f = files, e = fileEntries; f != files+fileCount; ++f, ++e) {
		if (!writeFile(*f, pak, e)) {
			pak->close();
			return false;
		}
	}

	pak->close();
	return true;
}

static void cleanup() {
	for (file_handle_t **f = files+fileCount; f-- != files;) (*f)->close();
}

static const char *readLine(file_handle_t *f) {
	static char buf[256];
	char *p = buf;

	do {
	l_ignore:
		if (f->read(p, 1) <= 0) return NULL;

		// Ignore \r
		if (*p == '\r') goto l_ignore;
	} while (*p++ != '\n');

	*--p = 0; // Replace new line with null terminator
	return buf;
}

static char *nextFilename(char *filename) {
	// +1 for NULL terminator
	return filename+1 + strlen(filename);
}

void gen_main(mem_t &m, file_system_t &f, const char *txt, const char *output) {
	mem = &m;
	sys = &f;

	file_handle_t *input = sys->open(txt, FILE_MODE_READ);
	if (!input) throw log_except("Couldn't open input file %s!", txt);

	char dest[512];
	strcpy(dest, output);
	strcat(dest, "data.pak");

	// Build filename table
	const char *line;
	uptr numFiles = 0; // Number of files in data.txt
	char name[50] = "files/"; // Read everything from "files/"
	while ((line = readLine(input))) {
		if (strlen(line) >= 44) {
			input->close();
			throw log_except("%s is too big!", line);
		}

		strcpy(name+6, line);

		if ((size_t)(curFilename-endFilenames) <= strlen(name)) {
			input->close();
			throw log_except("Ran out of filename memory! (increase FILENAMESLEN in gen/data/gen.cpp)");
		}
		
		strcpy(curFilename, name);
		curFilename = nextFilename(curFilename);

		++numFiles;
	}
	
	curOffset = sizeof(hdr_t) + sizeof(entry_t)*numFiles;

	curFilename = filenames;
	while (numFiles--) {
		if (!addFile(curFilename)) {
			input->close();
			cleanup();
			throw log_except("Cannot open %s!", name);
		}

		curFilename = nextFilename(curFilename);
	}

	if (!writePak(dest)) {
		input->close();
		cleanup();
		throw log_except("Cannot write %s!", txt);
	}

	input->close();
	cleanup();
}
