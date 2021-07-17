#include "types.h"
#include "game.h"

game_t::game_t(interfaces_t &i, const args_t &args) : m_i(i), m_a(args) {
	// Allocate game state
	m_state = (game_state_t*)m_i.mem.alloc(sizeof(game_state_t));

	// Set speed based on -spd parameter
	m_state->spd = 1.f/60.f;
	if (m_a.check(str_hash("-spd"))) m_state->spd *= 8.f;

	// Set x and y
	m_state->x = 0.f;
	m_state->y = 0.f;
}

game_t::~game_t() {
	// Free game state memory
	m_i.mem.free(m_state);
}

game_update_ret_t game_t::update() {
	if (m_i.input.k.pressed[KEYC_ESCAPE]) return GAME_UPDATE_CLOSE;
	
	if (m_i.input.k.down[KEYC_LEFT]) m_state->x -= m_state->spd;
	else if (m_i.input.k.down[KEYC_RIGHT]) m_state->x += m_state->spd;

	if (m_i.input.k.down[KEYC_DOWN]) m_state->y -= m_state->spd;
	else if (m_i.input.k.down[KEYC_UP]) m_state->y += m_state->spd;
	
	return GAME_UPDATE_CONTINUE;
}
