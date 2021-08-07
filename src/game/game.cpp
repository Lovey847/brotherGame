#include "types.h"
#include "game.h"

#include <math.h>

// Load map into game and renderer
static ubool loadMap(mem_t &m, pak_t &p, game_state_t &state, pak_entry_t *atlasEnt, str_hash_t mapName) {
  // Load map into renderer
  state.r.load = true;

  // Load map file
  pak_entry_t mapEnt = p.getEntry(mapName);
  if (mapEnt == PAK_INVALID_ENTRY) {
    log_warning("Cannot find map!");
    return false;
  }

  const map_file_t *map = (map_file_t*)p.mapEntry(mapEnt);
  if (!map) {
    log_warning("Cannot map map entry!");
    return false;
  }

  try {
    state.map.load(m, *map);
  } catch (const log_except_t &err) {
    log_warning("Cannot load map: %s", err.str());
    return false;
  }

  p.unmapEntry(mapEnt);

  // Load map atlas
  atlasEnt[ATLAS_LEVEL] = p.getEntry(state.map.levelAtlas);
  if (atlasEnt[ATLAS_LEVEL] == PAK_INVALID_ENTRY) {
    log_warning("Cannot find map atlas!");
    state.map.free(m);
    return false;
  }

  state.r.atlas[ATLAS_LEVEL] = (atlas_t*)p.mapEntry(atlasEnt[ATLAS_LEVEL]);
  if (!state.r.atlas[ATLAS_LEVEL]) {
    log_warning("Cannot map map atlas!");
    state.map.free(m);
    return false;
  }

  return true;
}

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
	m_state->pos = vec4(1024.f, 1024.f, 1024.f, 1.f);
  m_state->yaw = (f32)M_PI*0.5f;
  m_state->pitch = 0.f;

  // Set FOV, based on command-line parameters
  // NOTE: No error checking is done here!
  const f32 fov = str_strnum<f32>(m_a.valDef(str_hash("-fov"), "120"))*((f32)M_PI/180.f);
  m_state->fovy = fov * (9.f/16.f);

  // Initialize atlasEnt
  for (atlas_id_t i = 0; i < ATLAS_COUNT; ++i)
    m_atlasEnt[i] = PAK_INVALID_ENTRY;

  // Load resources into renderer
  m_state->r.load = true;

  // Load global atlas
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

  // Load map
  if (!loadMap(m_i.mem, m_pak, *m_state, m_atlasEnt, str_hash("maps/first.map"))) {
    m_pak.unmapEntry(m_atlasEnt[ATLAS_GLOBAL]);
    m_i.mem.free(m_state);
    throw log_except("Cannot load maps/first.map!");
  }

#ifdef GAME_STATE_EDITOR
  // Initialize editor vars
  m_state->blockDist = 512.f;
  m_state->blockSize = vec4(1.f, 1.f, 1.f, 0.f);
  m_state->blockGrid = vec4(64.f, 64.f, 64.f, 1.f);

  if (m_state->map.cubeCount >= 255) {
    m_state->map.free(m_i.mem);
    m_pak.unmapEntry(m_atlasEnt[ATLAS_LEVEL]);
    m_pak.unmapEntry(m_atlasEnt[ATLAS_GLOBAL]);
    m_i.mem.free(m_state);
    throw log_except("Map is too big!");
  }

  m_state->editorCubeCount = m_state->map.cubeCount;

  for (uptr i = 0; i < m_state->map.cubeCount; ++i) {
    m_state->editorCubes[i] = m_state->map.cubes[i];
  }

  m_state->map.free(m_i.mem);
#endif
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

#ifndef GAME_STATE_EDITOR
#else // GAME_STATE_EDITOR

static void writeMap(file_handle_t &out, game_state_t &state) {
  static const u8 zeros[sizeof(map_file_t)+sizeof(map_file_cube_t)*255] = {};

  out.write(zeros, sizeof(map_file_t)-sizeof(map_file_cube_t) + state.editorCubeCount*sizeof(map_file_cube_t));
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

  outMap->unmap();
}

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
  vec4 left = vec4(-s, 0.f, c, 0.f);

  // Fly around map
  if (m_i.input.k.down[KEYC_W]) m_state->pos += forward*8.f;
  if (m_i.input.k.down[KEYC_S]) m_state->pos -= forward*8.f;
  if (m_i.input.k.down[KEYC_A]) m_state->pos += left*8.f;
  if (m_i.input.k.down[KEYC_D]) m_state->pos -= left*8.f;

  forward *= vec4(cosf(m_state->pitch), 0.f, cosf(m_state->pitch), 0.f);
  forward += vec4(0.f, sinf(m_state->pitch), 0.f, 0.f);

  // Add & remove cubes
  if (m_i.input.k.pressed[KEYC_M_PRIMARY] && (m_state->editorCubeCount < 255)) ++m_state->editorCubeCount;
  else if (m_i.input.k.pressed[KEYC_M_SECONDARY] && (m_state->editorCubeCount > 0)) --m_state->editorCubeCount;

  // Switch cube texture
  if (m_i.input.k.pressed[KEYC_Q]) {
    if (m_state->blockTexture == 0) m_state->blockTexture = sizeof(game_state_texNames)/sizeof(str_hash_t)-1;
    else --m_state->blockTexture;
  } else if (m_i.input.k.pressed[KEYC_E]) {
    if (m_state->blockTexture == sizeof(game_state_texNames)/sizeof(str_hash_t)-1) m_state->blockTexture = 0;
    else ++m_state->blockTexture;
  }

  // Modifier keys
  if (m_i.input.k.down[KEYC_CTRL]) {

    // Change cube width
    if (m_i.input.k.pressed[KEYC_M_SCROLLUP]) m_state->blockSize.f[0] += 1.f;
    else if (m_i.input.k.pressed[KEYC_M_SCROLLDOWN]) m_state->blockSize.f[0] -= 1.f;

    // Move down
    if (m_i.input.k.down[KEYC_SPACE]) m_state->pos.f[1] -= 8.f;

  } else if (m_i.input.k.down[KEYC_ALT]) {

    // Change cube height
    if (m_i.input.k.pressed[KEYC_M_SCROLLUP]) m_state->blockSize.f[1] += 1.f;
    else if (m_i.input.k.pressed[KEYC_M_SCROLLDOWN]) m_state->blockSize.f[1] -= 1.f;

  } else if (m_i.input.k.down[KEYC_SHIFT]) {

    // Change cube depth
    if (m_i.input.k.pressed[KEYC_M_SCROLLUP]) m_state->blockSize.f[2] += 1.f;
    else if (m_i.input.k.pressed[KEYC_M_SCROLLDOWN]) m_state->blockSize.f[2] -= 1.f;

  } else {

    // Set cube distance from camera
    if (m_i.input.k.pressed[KEYC_M_SCROLLUP]) m_state->blockDist += m_state->blockGrid.f[0];
    else if (m_i.input.k.pressed[KEYC_M_SCROLLDOWN]) m_state->blockDist -= m_state->blockGrid.f[0];

    // Move up
    if (m_i.input.k.down[KEYC_SPACE]) m_state->pos.f[1] += 8.f;

  }

  // Change grid
  if (m_i.input.k.pressed[KEYC_OPENBRACKET]) m_state->blockGrid *= vec4(2.f, 2.f, 2.f, 1.f);
  else if (m_i.input.k.pressed[KEYC_CLOSEBRACKET]) m_state->blockGrid *= vec4(0.5f, 0.5f, 0.5f, 1.f);

  vec4 cubePos = m_state->pos + forward*m_state->blockDist;

  // Align cubePos to grid
  cubePos =
    vec4_ivec4(ivec4_vec4(cubePos/m_state->blockGrid))*m_state->blockGrid;

  m_state->editorCubes[m_state->editorCubeCount].min = cubePos;
  m_state->editorCubes[m_state->editorCubeCount].max =
    cubePos + m_state->blockSize*m_state->blockGrid;
  m_state->editorCubes[m_state->editorCubeCount].img = game_state_texNames[m_state->blockTexture];

  // Write map
  if (m_i.input.k.pressed[KEYC_F2]) {
    file_handle_t *out = m_i.fileSys.open("editor.map", FILE_MODE_READWRITE);
    if (out) writeMap(*out, *m_state);
    out->close();
  }

	return GAME_UPDATE_CONTINUE;
}

#endif // GAME_STATE_EDITOR
