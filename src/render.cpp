#include "types.h"
#include "mem.h"
#include "render.h"

ubool render_backend_t::expandRect(render_vertex_t out[4], const render_vertex_t v[2]) {
	// Verify triangles all share the same image
	if (v[0].img != v[1].img) {
		log_warning("Vertex images differ! (%u, %u)", v[0].img, v[1].img);
		return false;
	}

	out[0] = v[0];
	out[3] = v[1];

	// Generate top right and bottom left by merging top left and bottom right
	out[1].pos = v[0].pos.comp<0x1000>(v[1].pos);
	out[1].coord = out[0].coord.comp<0x1000>(v[1].coord);
	out[1].img = v[0].img;

	// Blend both colors
	// Round v[1].blend up
	out[1].blend.p = ((v[0].blend.p>>1)&0x7f7f7f7f) + ((v[1].blend.p>>1)&0x7f7f7f7f) + (v[1].blend.p&0x01010101);

	out[2].pos = v[0].pos.comp<0x0100>(v[1].pos);
	out[2].coord = out[0].coord.comp<0x0100>(v[1].coord);
	out[2].blend.p = out[1].blend.p; // Duplicate color calculation
	out[2].img = v[0].img;

	return true;
}
