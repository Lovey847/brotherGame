#include "types.h"
#include "game.h"

#include <math.h>

game_t::game_t(interfaces_t &i, const args_t &args) :
  m_i(i), m_a(args),

  // Check for pak file override
  m_pak(m_i.mem, m_i.fileSys, m_a.valDef(str_hash("-pak"), "data.pak"))
{
	// Allocate game state
	m_state = (game_state_t*)m_i.mem.alloc(sizeof(game_state_t));
  memset((void*)m_state, 0, sizeof(game_state_t));

  // Set module-state back pointers
  m_state->r.game = m_state;

  // Set position
	m_state->pos = vec4(-256.f, -256.f, 0.f, 1.f);

  // Set cube properties
  m_state->cube.min = vec4(-128.f, -384.f, -128.f, 1.f);
  m_state->cube.max = vec4(128.f, -128.f, 128.f, 1.f);
  m_state->cube.img = str_hash("xor");

  // Set FOV, based on command-line parameters
  const f32 fov = str_strnum<f32>(m_a.valDef(str_hash("-fov"), "120"))*((f32)M_PI/180.f);
  m_state->fovy = fov * (9.f/16.f);

  // Initialize atlasEnt
  for (atlas_id_t i = 0; i < ATLAS_COUNT; ++i)
    m_state->atlasEnt[i] = PAK_INVALID_ENTRY;

  // Load global atlas into renderer
  m_state->atlasEnt[ATLAS_GLOBAL] = m_pak.getEntry(str_hash("atlases/global.atl"));
  if (m_state->atlasEnt[ATLAS_GLOBAL] == PAK_INVALID_ENTRY)
    throw log_except("Cannot find atlases/global.atl!");

  m_state->r.atlas[ATLAS_GLOBAL] = (atlas_t*)m_pak.mapEntry(m_state->atlasEnt[ATLAS_GLOBAL]);
  if (!m_state->r.atlas) throw log_except("Cannot map atlases/global.atl!");
}

game_t::~game_t() {
  // Unmap atlases in render state
  for (atlas_id_t i = 0; i < ATLAS_COUNT; ++i) {
    if (m_state->atlasEnt[i] != PAK_INVALID_ENTRY) {
      m_pak.unmapEntry(m_state->atlasEnt[i]);
    }
  }

	// Free game state memory
	m_i.mem.free(m_state);
}

game_update_ret_t game_t::update() {
	if (m_i.input.k.pressed[KEYC_ESCAPE]) return GAME_UPDATE_CLOSE;

  if (m_i.input.k.down[KEYC_W]) m_state->pos.f[2] += 4.f;
  if (m_i.input.k.down[KEYC_S]) m_state->pos.f[2] -= 4.f;
  if (m_i.input.k.down[KEYC_A]) m_state->pos.f[0] -= 4.f;
  if (m_i.input.k.down[KEYC_D]) m_state->pos.f[0] += 4.f;
  if (m_i.input.k.down[KEYC_SPACE]) m_state->pos.f[1] += 4.f;
  if (m_i.input.k.down[KEYC_LCTRL]) m_state->pos.f[1] -= 4.f;

	return GAME_UPDATE_CONTINUE;
}
