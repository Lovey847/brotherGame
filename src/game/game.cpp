#include "types.h"
#include "game.h"

game_t::game_t(interfaces_t &i, const args_t &args) :
  m_i(i), m_a(args),

  // Check for pak file override
  m_pak(m_i.mem, m_i.fileSys, m_a.valDef(str_hash("-pak"), "data.pak"))
{
	// Allocate game state
	m_state = (game_state_t*)m_i.mem.alloc(sizeof(game_state_t));
  memset(m_state, 0, sizeof(game_state_t));

	// Set speed based on -spd parameter
	m_state->spd = 1.f/60.f;
	if (m_a.check(str_hash("-spd"))) m_state->spd *= 8.f;

	// Set x and y
	m_state->x = 0.f;
	m_state->y = 0.f;

  // Load global atlas into renderer
  m_state->r.atlasEnt[ATLAS_GLOBAL] = m_pak.getEntry(str_hash("atlases/global.atl"));
  if (m_state->r.atlasEnt[ATLAS_GLOBAL] == PAK_INVALID_ENTRY)
    throw log_except("Cannot find atlases/global.atl!");

  m_state->r.atlas[ATLAS_GLOBAL] = (atlas_t*)m_pak.mapEntry(m_state->r.atlasEnt[ATLAS_GLOBAL]);
  if (!m_state->r.atlas) throw log_except("Cannot map atlases/global.atl!");
}

game_t::~game_t() {
  // Unmap atlases in render state
  for (atlas_id_t i = 0; i < ATLAS_COUNT; ++i) {
    if (m_state->r.atlas[i]) {
      m_pak.unmapEntry(m_state->r.atlasEnt[i]);
      m_state->r.atlasEnt[i] = PAK_INVALID_ENTRY;
      m_state->r.atlas[i] = NULL;
    }
  }

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
