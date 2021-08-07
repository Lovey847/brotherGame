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

  // Set initial position and direction
	m_state->pos = vec4(0.f, 0.f, -256.f, 1.f);
  m_state->yaw = (f32)M_PI*0.5f;
  m_state->pitch = 0.f;

  // Set FOV, based on command-line parameters
  // NOTE: No error checking is done here!
  const f32 fov = str_strnum<f32>(m_a.valDef(str_hash("-fov"), "120"))*((f32)M_PI/180.f);
  m_state->fovy = fov * (9.f/16.f);

  // Initialize atlasEnt
  for (atlas_id_t i = 0; i < ATLAS_COUNT; ++i)
    m_atlasEnt[i] = PAK_INVALID_ENTRY;

  // Load global atlas into renderer
  m_state->r.load = true;

  m_atlasEnt[ATLAS_GLOBAL] = m_pak.getEntry(str_hash("atlases/global.atl"));
  if (m_atlasEnt[ATLAS_GLOBAL] == PAK_INVALID_ENTRY) {
    m_i.mem.free(m_state);
    throw log_except("Cannot find atlases/global.atl!");
  }

  m_state->r.atlas[ATLAS_GLOBAL] = (atlas_t*)m_pak.mapEntry(m_atlasEnt[ATLAS_GLOBAL]);
  if (!m_state->r.atlas) {
    m_i.mem.free(m_state);
    throw log_except("Cannot map atlases/global.atl!");
  }

  // Load map as well
  pak_entry_t mapEnt = m_pak.getEntry(str_hash("maps/first.map"));
  if (mapEnt == PAK_INVALID_ENTRY) {
    m_i.mem.free(m_state);
    m_pak.unmapEntry(m_atlasEnt[ATLAS_GLOBAL]);
    throw log_except("Cannot find maps/first.map!");
  }

  const map_file_t *map = (map_file_t*)m_pak.mapEntry(mapEnt);
  if (!map) {
    m_i.mem.free(m_state);
    m_pak.unmapEntry(m_atlasEnt[ATLAS_GLOBAL]);
    throw log_except("Cannot map maps/first.map!");
  }

  m_state->map.load(m_i.mem, *map);

  m_pak.unmapEntry(mapEnt);
}

game_t::~game_t() {
  // Unmap atlases in render state
  for (atlas_id_t i = 0; i < ATLAS_COUNT; ++i) {
    if (m_atlasEnt[i] != PAK_INVALID_ENTRY) {
      m_pak.unmapEntry(m_atlasEnt[i]);
    }
  }

  // Free map memory
  m_state->map.free(m_i.mem);

	// Free game state memory
	m_i.mem.free(m_state);
}

#ifdef GAME_STATE_EDITOR

#if 0
static void writeMap(file_handle_t &out, game_state_t &state) {
  static const u8 zeros[sizeof(map_file_t)+sizeof(map_file_cube_t)*255] = {};

  out.write(zeros, state.editorCubeCount*sizeof(map_file_cube_t));
  file_mapping_t *outMap = out.map(FILE_MAP_READWRITE, 0, state.editorCubeCount*sizeof(map_file_cube_t));
  if (!outMap) throw log_except("Couldn't map output map file!");

  map_file_t *map = (map_file_t*)outMap->data;

  map->magic = MAP_MAGIC;
  map->cubeCount = state.editorCubeCount;
  map->prevLoad.min = map->prevLoad.max = map->nextLoad.min = map->nextLoad.max = vec4(0.f); // TODO: Load a value into these!
  map->prevLoad.map = GAME_STATE_PREVMAP;
  map->nextLoad.map = GAME_STATE_NEXTMAP;

  for (uptr i = 0; i < state.editorCubeCount; ++i) {
    map->cubes[i].min = state.editorCubes[i].min;
    map->cubes[i].max = state.editorCubes[i].max;
    map->cubes[i].img = state.editorCubes[i].img;
  }

  map->levelAtlas = GAME_STATE_LEVELATLAS;
}
#endif

game_update_ret_t game_t::update() {
	if (m_i.input.k.pressed[KEYC_ESCAPE]) return GAME_UPDATE_CLOSE;

  m_state->yaw += (f32)((i32)m_state->w.width/2-m_i.input.mx)*0.005f;
  m_state->pitch += (f32)((i32)m_state->w.height/2-m_i.input.my)*0.005f;

  while (m_state->yaw < 0.f) m_state->yaw += (f32)M_PI*2.f;
  while (m_state->yaw > (f32)M_PI*2.f) m_state->yaw -= (f32)M_PI*2.f;

  if (m_state->pitch < -(f32)M_PI/2.f) m_state->pitch = -(f32)M_PI/2.f;
  else if (m_state->pitch > (f32)M_PI/2.f) m_state->pitch = (f32)M_PI/2.f;

  const f32 c = cosf(m_state->yaw);
  const f32 s = sinf(m_state->yaw);

  vec4 forward = vec4(c, 0.f, s, 0.f);
  forward *= vec4(cosf(m_state->pitch), 0.f, cosf(m_state->pitch), 0.f);
  forward += vec4(0.f, sinf(m_state->pitch), 0.f, 0.f);

  vec4 left = vec4(-s, 0.f, c, 0.f);

  if (m_i.input.k.down[KEYC_W]) m_state->pos += forward*8.f;
  if (m_i.input.k.down[KEYC_S]) m_state->pos -= forward*8.f;
  if (m_i.input.k.down[KEYC_A]) m_state->pos += left*8.f;
  if (m_i.input.k.down[KEYC_D]) m_state->pos -= left*8.f;

  if (m_i.input.k.pressed[KEYC_M_PRIMARY] && (m_state->editorCubeCount < 255)) ++m_state->editorCubeCount;
  else if (m_i.input.k.pressed[KEYC_M_SECONDARY] && (m_state->editorCubeCount > 0)) --m_state->editorCubeCount;

  if (m_i.input.k.pressed[KEYC_DOWN]) {
    if (m_state->blockTexture == 0) m_state->blockTexture = sizeof(game_state_texNames)/sizeof(str_hash_t)-1;
    else --m_state->blockTexture;
  } else if (m_i.input.k.pressed[KEYC_UP]) {
    if (m_state->blockTexture == sizeof(game_state_texNames)/sizeof(str_hash_t)-1) m_state->blockTexture = 0;
    else ++m_state->blockTexture;
  }

  if (m_i.input.k.pressed[KEYC_M_SCROLLUP]) m_state->blockDist += 8.f;
  else if (m_i.input.k.pressed[KEYC_M_SCROLLDOWN]) m_state->blockDist -= 8.f;

  m_state->editorCubes[m_state->editorCubeCount].min =
    m_state->pos + forward*m_state->blockDist - vec4(128.f, 128.f, 128.f, 0.f);
  m_state->editorCubes[m_state->editorCubeCount].max =
    m_state->pos + forward*m_state->blockDist + vec4(128.f, 128.f, 128.f, 0.f);
  m_state->editorCubes[m_state->editorCubeCount].img = game_state_texNames[m_state->blockTexture];

	return GAME_UPDATE_CONTINUE;
}

#else // MAP_EDITOR

#endif // MAP_EDITOR
