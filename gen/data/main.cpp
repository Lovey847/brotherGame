// Data generator: Parse all command-line arguments as file paths, and packs every file into data.pak

#include "types.h"
#include "str.h"
#include "util.h"
#include "endianUtil.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

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
static FILE **files;
static entry_t *fileEntries;
static uptr maxFiles = 256, fileCount = 0;

static u32 curOffset;

static ubool addFile(const char *fname) {
	FILE *f = fopen(fname, "rb");
	if (!f) return false;
	
	fseek(f, 0, SEEK_END);
	uptr fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	// Resize buffers if necessary
	if (fileCount >= maxFiles) {
		maxFiles *= 2;
		
		files = (FILE**)realloc(files, sizeof(FILE*)*maxFiles);
		fileEntries = (entry_t*)realloc(fileEntries, sizeof(entry_t)*maxFiles);
	}

	files[fileCount] = f;

	entry_t *e = fileEntries + fileCount;

	memcpy(e->name, fname, strlen(fname));
	e->hash = str_hashR(e->name); // Hashing at runtime, slow operation
	e->offset = curOffset;
	e->size = fileSize;

	// Align all data to a 16-byte boundary
	curOffset += util_alignUp<u32>(e->size, 16);
	++fileCount;

	return true;
}

static ubool writeFile(FILE *f, FILE *pak, entry_t *e) {
	u8 buf[2048];

	uptr fileSize = util_alignUp<u32>(e->size, 16);
	while (fileSize) {
		const uptr size = util_min<uptr>(fileSize, sizeof(buf));
		fread(buf, 1, size, f);
		fwrite(buf, 1, size, pak);

		fileSize -= size;
	}

	return true;
}

static ubool writePak(const char *fname) {
	FILE **f;
	entry_t *e;

	FILE *pak;

	hdr_t hdr;

	pak = fopen(fname, "wb");
	if (!pak) return false;

	hdr.magic = util_magic('P', 'A', 'K', '2');
	hdr.entryCount = fileCount;
	fwrite(&hdr, 1, sizeof(hdr_t), pak);

	fwrite(fileEntries, 1, sizeof(entry_t)*fileCount, pak);

	for (f = files, e = fileEntries; f != files+fileCount; ++f, ++e) {
		if (!writeFile(*f, pak, e)) {
			fclose(pak);
			return false;
		}
	}

	fclose(pak);
	return true;
}

static void cleanup() {
	for (FILE **f = files+fileCount; f-- != files;)
		fclose(*f);

	free(files);
	free(fileEntries);
}

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("Usage: %s <filename1> <filename2> ... <destination file>\n"
			   "\n"
			   "filename#        - Filename of next file to pack into pak file\n"
			   "destination file - Filename of pak file\n",

			   argv[0]);
		return 0;
	}

	const char * const dest = argv[--argc];

	files = (FILE**)malloc(sizeof(FILE*)*maxFiles);
	fileEntries = (entry_t*)malloc(sizeof(entry_t)*maxFiles);

	curOffset = sizeof(hdr_t) + sizeof(entry_t)*argc - sizeof(entry_t);

	while (--argc) {
		if (!addFile(argv[argc])) {
			printf("Cannot open %s!\n", argv[argc]);
			cleanup();
			return 1;
		}
	}

	if (!writePak(dest)) {
		printf("Cannot write %s!", dest);
		return 1;
	}

	cleanup();

	return 0;
}
