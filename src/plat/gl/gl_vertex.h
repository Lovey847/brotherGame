#ifndef GL_VERTEX_H
#define GL_VERTEX_H

#include "types.h"
#include "gl_texture.h"

struct gl_vertex_t {
	FINLINE gl_vertex_t() {}

	FINLINE gl_vertex_t(const gl_texture_t &tex, const render_vertex_t &other) {
		pos = other.pos;
		coord = tex.imgOffset(other.img);
		coord.x += other.coord.x;
		coord.y += other.coord.y;
		col.p = other.blend.p;

		// Set defaults for object parameters
		col2.p = 0xffffffff; // Default second blend
		dir() = 0.f;
		offsetScale = vec4(0.f, 0.f, 1.f, 1.f);

		// Old values
		oldCol2.p = 0xffffffff;
		oldDir() = 0.f;
		oldOffsetScale = vec4(0.f, 0.f, 1.f, 1.f);
	}

	// Update vertex with object parameters
	// Set old components with old
	FINLINE void update(const render_objProp_t &prop, const render_objProp_t &old) {
		col2.p = prop.col.p;
		dir() = prop.dir;
		offsetScale = prop.posScale;

		oldCol2.p = old.col.p;
		oldDir() = old.dir;
		oldOffsetScale = old.posScale;
	}
};

// Make sure we're correct in our size assessment here
static_assert(sizeof(gl_vertex_t) == 64, "");

#endif //GL_VERTEX_H
