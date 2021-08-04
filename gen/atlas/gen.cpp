#include "gen.h"
#include "str.h"
#include "util.h"
#include "endianUtil.h"
#include "file.h"
#include "log.h"
#include "atlas.h"

#include <cstring>

static file_system_t *sys;
static mem_t *mem;

// Yeah, I know this is a compiler extension
// I also gotta write this fast
#pragma pack(1)

struct bmpHdr_t {
  char magic[2]; // == 'B', 'M'

  u32 size; // Size of file
  u16 reserved[2];
  u32 offset; // Offset to pixels from start of file

  u32 hdrSize;
  u32 width; // == ATLAS_WIDTH
  u32 height; // == ATLAS_HEIGHT
  u16 planes; // == 1
  u16 bpp; // == 32
};

#pragma pack()

// atlas_t, without imgNames field
struct hdr_t {
  u32 magic;
  endian_u32 imageCount;
  pak_ptr_t<atlas_col_t> data;
  pak_ptr_t<endian_ivec4> imgDim;
};

static constexpr uptr MAXIMG = 256;
static atlas_col_t data[ATLAS_WIDTH*ATLAS_HEIGHT];
static str_hash_t imgNames[MAXIMG], *curImgName = imgNames;
static endian_ivec4 imgDim[MAXIMG], *curDim = imgDim;

// Read image data from BMP file
static void readData(file_handle_t *bmp) {
  bmpHdr_t hdr;
  if (bmp->read(&hdr, sizeof(bmpHdr_t)) < (iptr)sizeof(bmpHdr_t))
    throw log_except("Cannot read %d bytes from BMP!", (int)sizeof(bmpHdr_t));

  // Input sanitization
  if ((hdr.magic[0] != 'B') ||
      (hdr.magic[1] != 'M') ||
      (hdr.width != ATLAS_WIDTH) ||
      (hdr.height != ATLAS_HEIGHT) ||
      (hdr.planes != 1) ||
      (hdr.bpp != 32)) throw log_except("Invalid BMP header!");

  // File seems good, map data
  file_mapping_t *flippedMapping = bmp->map(FILE_MAP_READ, hdr.offset, ATLAS_WIDTH*ATLAS_HEIGHT*4);
  if (!flippedMapping)
    throw log_except("Cannot map BMP image data!");

  const atlas_col_t *flippedData = (const atlas_col_t*)(flippedMapping->data);

  // Flip data vertically
  for (uptr i = 0; i < ATLAS_HEIGHT; ++i) {
    u8 tmp;
    atlas_col_t *ptr1 = data + i*ATLAS_WIDTH;
    const atlas_col_t *ptr2 = flippedData + (ATLAS_HEIGHT-1-i)*ATLAS_WIDTH;

    memcpy(ptr1, ptr2, ATLAS_WIDTH*4);

    // Atlas image data is in RGBA, convert BMP's BGRA to RGBA
    for (uptr x = 0; x < ATLAS_WIDTH; ++x) {
      tmp = ptr1[x].c[0];
      ptr1[x].c[0] = ptr1[x].c[2];
      ptr1[x].c[2] = tmp;
    }
  }

  flippedMapping->unmap();
}

// Read line from atlas.txt
static const char *readLine(file_handle_t *f) {
	static char buf[256];
	char *p = buf;

	do {
	l_ignore:
		if (f->read(p, 1) <= 0) throw log_except("Cannot read line from atlas.txt!");

		// Ignore \r
		if (*p == '\r') goto l_ignore;
	} while (*p++ != '\n');

	*--p = 0; // Replace new line with null terminator
	return buf;
}

// Read image from atlas.txt
static void readImage(file_handle_t *txt) {
  // Read name
  *curImgName++ = str_hashR(readLine(txt));

  // Read dimensions
  ivec4 dim;

  dim.i[0] = str_strnum<i32>(readLine(txt));
  dim.i[1] = str_strnum<i32>(readLine(txt));
  dim.i[2] = str_strnum<i32>(readLine(txt));
  dim.i[3] = str_strnum<i32>(readLine(txt));

  *curDim++ = dim;
}

// Read atlas from atlas.txt
static void readAtlas(file_handle_t *txt) {
  // Read atlas name
  char name[256];
  strcpy(name, readLine(txt));

  // Read number of images in atlas
  uptr imageCount = str_strnum<uptr>(readLine(txt));

  // Read atlas data
  const char *tgaName = readLine(txt);
  file_handle_t *tga = sys->open(tgaName, FILE_MODE_READ);
  if (!tga) throw log_except("Cannot open %s!", tgaName);

  readData(tga);
  tga->close();

  // Read images from atlas
  uptr curImg = imageCount;
  while (curImg--) readImage(txt);

  // Output atlas to file
  file_handle_t *out = sys->open(name, FILE_MODE_WRITE);
  if (!out) throw log_except("Cannot write to %s!", name);

  hdr_t hdr;
  const uptr dataOffset = sizeof(hdr) + sizeof(str_hash_t)*imageCount;
  const uptr imgDimOffset = util_alignUp<uptr>(dataOffset + 4*ATLAS_WIDTH*ATLAS_HEIGHT, 16);

  hdr.magic = ATLAS_MAGIC;
  hdr.imageCount = imageCount;
  hdr.data.set((u8*)&hdr + dataOffset);
  hdr.imgDim.set((u8*)&hdr + imgDimOffset);

  // Write header
  out->write(&hdr, sizeof(hdr));

  // Write image names
  out->write(imgNames, sizeof(str_hash_t)*(curImgName-imgNames));

  // Write image data
  out->write(data, ATLAS_WIDTH*ATLAS_HEIGHT*4);

  // Write padding before image dimensions
  const u8 zeros[15] = {};
  out->write(zeros, imgDimOffset - (dataOffset + 4*ATLAS_WIDTH*ATLAS_HEIGHT));

  // Write image dimensions
  out->write(imgDim, 32*(curDim-imgDim));

  // Close output
  out->close();
}

void gen_main(mem_t &m, file_system_t &f, const char *txtName, const char *output) {
  mem = &m;
  sys = &f;

  (void)output;

  file_handle_t *txt = sys->open(txtName, FILE_MODE_READ);
  if (!txt) throw log_except("Can't open %s!", txtName);

  // Write all atlases
  uptr atlasCount = str_strnum<uptr>(readLine(txt));
  while (atlasCount--) readAtlas(txt);

  txt->close();
}
