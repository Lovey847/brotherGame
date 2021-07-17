#include "types.h"
#include "util.h"
#include "gl_texture.h"
#include "gl_buffer.h"
#include "gl_glf.h"

#include <cstring>

struct gl_texture_cell_page_t::cell_t {
	u32 width, height;

	constexpr cell_t(u32 w, u32 h) : width(w), height(h) {}

	constexpr ubool active() const {return width != 0;}
};

void gl_texture_cell_page_t::init(mem_t &m, uptr width, uptr height) {
	if (active()) destroy();

	m_cells = (cell_t*)m.alloc(sizeof(cell_t) * width*height);
	memset((void*)m_cells, 0, sizeof(cell_t) * width*height);

	// The cell of the white square is always active
	m_cells[0] = cell_t(1, 1);

	m_width = width;
	m_height = height;

	m_m = &m;
}

void gl_texture_cell_page_t::destroy() {
	m_m->free(m_cells);
}

iptr gl_texture_cell_page_t::checkImg(iptr x, iptr y, u32 width, u32 height) const {
	// Iterate over x positions, left to right
	const iptr xEnd = x+width;
	
	for (iptr xx = x; xx != xEnd; ++xx) {
		// Iterate over y positions, top to bottom
		const cell_t * const end = m_cells + (y+height-1)*m_width + xx;
		
		for (
			const cell_t *cell = m_cells + y*m_width + xx;
			cell != end; cell += m_width
		) {
			// If active cell found, return offset
			if (cell->active()) return xx-xEnd;
		}
	}

	// Image fits, return 0
	return 0;
}

void gl_texture_cell_page_t::markImg(iptr x, iptr y, u32 width, u32 height) {
	const cell_t cell(width, height);
	
	// Iterate over width and height of cell
	for (iptr yy = y+height; yy != y;) {
		--yy;

		for (iptr xx = x+width; xx != x;) {
			--xx;

			m_cells[yy*m_width + xx] = cell;
		}
	}
}

ubool gl_texture_cell_page_t::packImg(u32 width, u32 height, u32 *out) {
	// Start at bottom right corner, work up to top left
	iptr x = m_width-width, y = m_height-height;
	iptr pos;

	for (;;) {
		pos = checkImg(x, y, width, height);
		if (pos == 0) break;

		if (x+pos < 0) {
			if (y == 0) return false;
			
			--y;
			x = m_width-width;
		} else x += pos;
	}

	out[0] = x;
	out[1] = y;

	markImg(x, y, width, height);

	return true;
}

void gl_texture_cell_page_t::removeImg(u32 x, u32 y) {
	for (uptr yy = y+m_cells[y*m_width + x].height; yy != y;) {
		--yy;

		for (uptr xx = x+m_cells[y*m_width + x].width; xx != x;) {
			--xx;

			// Mark cell as inactive
			m_cells[yy*m_width + xx].width = 0;
		}
	}
}

void gl_texture_cell_page_t::clear() {
	memset((void*)(m_cells+1), 0, sizeof(cell_t) * m_width*m_height - sizeof(cell_t));
}

void gl_texture_t::init(mem_t &m, pak_t &p, uptr size) {
	if (active()) destroy();

	m_cells.init(m, size/GLTEXTURE_CELLSIZE, size/GLTEXTURE_CELLSIZE);
	
	const uptr memSize = size*size*4;

	// Allocate temporary buffer that includes RENDER_IMAGE_WHITE in the top left
	// and transparency everywhere else
	render_color_t * const mem = (render_color_t*)m.allocTemp(memSize);
	
	for (uptr i = memSize/4-1; i; --i) {
		if (((i%size) >> 1) == 0) mem[i] = 0xffffffff;
		else mem[i] = 0x00000000;
	}

	// Create unpack buffer
	GLF(GL::GenBuffers(1, &m_unpack));
	GLF(GL::BindBuffer(GL::PIXEL_UNPACK_BUFFER, m_unpack));
	GLF(GL::BufferData(GL::PIXEL_UNPACK_BUFFER, memSize, mem, GL::STATIC_DRAW));

	// Set image count
	m_imageCount = 0;

	m_size = size;

	// Create texture
	GLF(GL::GenTextures(1, &m_tex));
	GLF(GL::BindTexture(GL::TEXTURE_2D, m_tex));

	// Load m_unpack into m_tex
	GLF(GL::TexImage2D(GL::TEXTURE_2D, 0,
					   GL::RGBA8, size, size,
					   0, GL::BGRA, GL::UNSIGNED_BYTE, NULL));
	GLF(GL::BindBuffer(GL::PIXEL_UNPACK_BUFFER, 0)); // unbind m_unpack

	GLF(GL::TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MIN_FILTER, GL::NEAREST));
	GLF(GL::TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MAG_FILTER, GL::NEAREST));
	GLF(GL::TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_WRAP_S, GL::CLAMP_TO_EDGE));
	GLF(GL::TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_WRAP_T, GL::CLAMP_TO_EDGE));

	// Allocate m_imageOffsets for RENDER_IMAGE_WHITE
	m_imageOffset = (gl_texture_coord_t*)m.alloc(sizeof(gl_texture_coord_t));
	m_imageOffset[RENDER_IMAGE_WHITE].x = 0;
	m_imageOffset[RENDER_IMAGE_WHITE].y = 0;

	m_m = &m;
	m_p = &p;
}

void gl_texture_t::destroy() {
	// Free pixel unpack buffer
	GLF(GL::DeleteBuffers(1, &m_unpack));

	// Free image offsets, if existent
	if (m_imageOffset) {
		m_m->free(m_imageOffset);
		m_imageOffset = NULL;
	}

	// Free texture
	GLF(GL::DeleteTextures(1, &m_tex));

	// Reset m_m
	m_m = NULL;
}

ubool gl_texture_t::loadImg(uptr imgCount, const str_hash_t *img) {
	ivec2_2 pos;

	m_imageCount = imgCount;

	// Reallocate m_imageOffset for new image
	// Include white square in allocation
	m_m->free(m_imageOffset);
	m_imageOffset = (gl_texture_coord_t*)m_m->alloc(sizeof(gl_texture_coord_t)*imgCount +
													sizeof(gl_texture_coord_t));
	
	m_imageOffset[RENDER_IMAGE_WHITE].x = 0;
	m_imageOffset[RENDER_IMAGE_WHITE].y = 0;

	// Clear texture
	GLF(GL::BindBuffer(GL::PIXEL_UNPACK_BUFFER, m_unpack));
	GLF(GL::TexSubImage2D(GL::TEXTURE_2D, 0, 0, 0, m_size, m_size, GL::BGRA, GL::UNSIGNED_BYTE, NULL));
	GLF(GL::BindBuffer(GL::PIXEL_UNPACK_BUFFER, 0));

	// Clear cell page
	m_cells.clear();

	// Iterate over images, loading them into the texture, cell page and image offsets
	while (imgCount) {
		// Open entry
		pak_entry_t *ent = m_p->open(img[imgCount-1]);
		if (!ent) {
			log_warning("Unable to load image %u!", (u32)imgCount-1);

			--imgCount;
			continue;
		}
		
		const render_image_t * const img = (const render_image_t*)ent->data;

		if (img->magic != RENDER_IMAGE_MAGIC) {
			log_warning("Invalid image %u!", (u32)imgCount);
			ent->close();
			continue;
		}
		
		// If unable to load into cell page, return false
		if (!m_cells.packImg(util_divUp(img->width+GLTEXTURE_CELLMARGIN, GLTEXTURE_CELLSIZE),
							 util_divUp(img->height+GLTEXTURE_CELLMARGIN, GLTEXTURE_CELLSIZE), (u32*)&pos))
			return false;

		// Multiply pos
		pos *= GLTEXTURE_CELLSIZE;

		// Load into offset table
		m_imageOffset[imgCount].x = pos.x;
		m_imageOffset[imgCount].y = pos.y;
		
		log_note("%hu %hu",
				 m_imageOffset[imgCount].x,
				 m_imageOffset[imgCount].y);

		// Load into texture
		GLF(GL::TexSubImage2D(GL::TEXTURE_2D, 0,
							  pos.x, pos.y, img->width, img->height,
							  GL::BGRA, GL::UNSIGNED_BYTE, img->data()));

		ent->close();

		--imgCount;
	}

	return true;
}
