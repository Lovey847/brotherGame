#ifndef GL_VERTEX_H
#define GL_VERTEX_H

#include "types.h"
#include "render.h"
#include "gl_texture.h"

// A single vertex is currently 64-bytes, unsure if that's too big for a 2D game or not
struct gl_vertex_t {
	vec2_2 pos; // Z & W components hold direction and old direction
	
	gl_texture_coord_t coord; // Texture coordinates
	render_color_t col; // Vertex color
	render_color_t col2; // Object color
	render_color_t oldCol2; // Old object color
	
	vec2_2 offsetScale;
	vec2_2 oldOffsetScale;
	
	FINLINE const f32 &dir() const {
		return pos.z;
	}
	FINLINE f32 &dir() {
		return pos.z;
	}

	FINLINE const f32 &oldDir() const {
		return pos.w;
	}
	FINLINE f32 &oldDir() {
		return pos.w;
	}

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
