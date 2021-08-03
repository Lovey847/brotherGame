#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#include "types.h"
#include "mem.h"
#include "opengl.h"
#include "game/pak.h"
#include "log.h"

struct gl_texture_coord_t {
	u16 x, y;
};

// The texture is a grid of cells, cells are GLTEXTURE_CELLSIZExGLTEXTURE_CELLSIZE,
// this means when packing images I can take a more brute force approach,
// but at a low cost since I'm traversing at a much lower resolution
static constexpr u32 GLTEXTURE_CELLSIZE = 32;

// Minimum pixel margin to have inbetween images in texture page
static constexpr u32 GLTEXTURE_CELLMARGIN = 2;

class gl_texture_cell_page_t {
private:
	struct cell_t;
	
	mem_t *m_m = NULL;

	// Cell buffer
	cell_t *m_cells;

	// Width & height
	uptr m_width, m_height;

	void destroy();

	// Returns offset to x position of rectangle while searching for free cells
	// If spot is free, return 0
	iptr checkImg(iptr x, iptr y, u32 width, u32 height) const;

	// Put image into cell page
	void markImg(iptr x, iptr y, u32 width, u32 height);

public:
	FINLINE gl_texture_cell_page_t() : m_m(NULL) {}
	FINLINE gl_texture_cell_page_t(mem_t &m, uptr width, uptr height) {
		init(m, width, height);
	}

	void init(mem_t &m, uptr width, uptr height);

	FINLINE gl_texture_cell_page_t(const gl_texture_cell_page_t&) = delete;

	FINLINE ~gl_texture_cell_page_t() {
		if (active()) destroy();
	}

	FINLINE ubool active() const {return m_m != NULL;}

	// Pack image into cell page, position is returned as 2 floats into pos
	// Returns false if image packing failed
	ubool packImg(u32 width, u32 height, u32 *out);

	// Remove image from cell page
	void removeImg(u32 x, u32 y);

	// Clear all cells in cell page
	void clear();
};

class gl_texture_t {
private:
	mem_t *m_m = NULL;
	pak_t *m_p;

	// PIXEL_UNPACK_BUFFER
	GLuint m_unpack;

	// Texture object
	GLuint m_tex;

	// Offset in texture to image
	gl_texture_coord_t *m_imageOffset;
	
	uptr m_imageCount; // Number of images stored in texture

	// Cell grid
	gl_texture_cell_page_t m_cells;
	
	uptr m_size; // width&height

	void destroy();
	
public:
	FINLINE gl_texture_t() {}
	FINLINE gl_texture_t(mem_t &m, pak_t &p, uptr size) {
		init(m, p, size);
	}
	FINLINE ~gl_texture_t() {
		if (active()) destroy();
	}

	FINLINE gl_texture_t(const gl_texture_t&) = delete;

	void init(mem_t &m, pak_t &p, uptr size);

	FINLINE ubool active() const {return m_m != NULL;}

	// Images are not changed if loadImg fails
	ubool loadImg(uptr imgCount, const str_hash_t *img);

	// Get offset in texture for image
	FINLINE const gl_texture_coord_t &imgOffset(render_imgId_t img) const {
		log_assert(img <= m_imageCount, "Invalid image index!");
		return m_imageOffset[img];
	}
};

#endif //GL_TEXTURE_H
