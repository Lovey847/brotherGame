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
    log_warning("Cannot map level entry!");
    return false;
  }

  try {
    state.map.load(m, *map);
  } catch (const log_except_t &err) {
    log_warning("Cannot load level: %s", err.str());
    return false;
  }

  p.unmapEntry(mapEnt);

  // Free previous map atlas
  if (atlasEnt[ATLAS_LEVEL] != PAK_INVALID_ENTRY) p.unmapEntry(atlasEnt[ATLAS_LEVEL]);

#ifndef GAME_STATE_EDITOR

  // Load map atlas
  atlasEnt[ATLAS_LEVEL] = p.getEntry(state.map.levelAtlas);
  if (atlasEnt[ATLAS_LEVEL] == PAK_INVALID_ENTRY) {
    log_warning("Cannot find level atlas!");
    state.map.free(m);
    return false;
  }

  state.r.atlas[ATLAS_LEVEL] = (atlas_t*)p.mapEntry(atlasEnt[ATLAS_LEVEL]);
  if (!state.r.atlas[ATLAS_LEVEL]) {
    log_warning("Cannot map level atlas!");
    state.map.free(m);
    return false;
  }

#else

  // Load map atlas
  atlasEnt[ATLAS_LEVEL] = p.getEntry(state.curMap->prop->atlas);
  if (atlasEnt[ATLAS_LEVEL] == PAK_INVALID_ENTRY) {
    log_warning("Cannot find level atlas!");
    state.map.free(m);
    return false;
  }

  state.r.atlas[ATLAS_LEVEL] = (atlas_t*)p.mapEntry(atlasEnt[ATLAS_LEVEL]);
  if (!state.r.atlas[ATLAS_LEVEL]) {
    log_warning("Cannot map level atlas!");
    state.map.free(m);
    return false;
  }

  // If map is empty, load map into editor
  if (state.curMap->cubeCount == 0) {
    if (state.map.cubeCount >= 255) {
      log_warning("Cannot load map into editor!");
      state.map.free(m);
      p.unmapEntry(atlasEnt[ATLAS_LEVEL]);
      return false;
    }

    state.curMap->cubeCount = state.map.cubeCount;

    for (uptr i = 0; i < state.map.cubeCount; ++i)
      state.curMap->cubes[i] = state.map.cubes[i];

    state.curMap->prevLoad = state.map.prevLoad;
    state.curMap->nextLoad = state.map.nextLoad;
  }

  // Free map, we don't need it
  state.map.free(m);

#endif

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
	m_state->pos = vec4(4096.f, 4096.f, 4096.f, 1.f);
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

#ifdef GAME_STATE_EDITOR

  // Initialize editor vars
  m_state->blockDist = 512.f;
  m_state->blockSize = vec4(1.f, 1.f, 1.f, 0.f);
  m_state->blockGrid = vec4(64.f, 64.f, 64.f, 1.f);

  // Load editor maps
  for (uptr i = GAME_STATE_MAPCOUNT; i--;) {
    m_state->curMap = m_state->maps+i;
    m_state->curMap->prop = game_state_maps+i;

    if (!loadMap(m_i.mem, m_pak, *m_state, m_atlasEnt, m_state->curMap->prop->hashName)) {
      m_pak.unmapEntry(m_atlasEnt[ATLAS_GLOBAL]);
      m_i.mem.free(m_state);
      throw log_except("Cannot load map into editor!");
    }
  }

#else

  // Load first map
  if (!loadMap(m_i.mem, m_pak, *m_state, m_atlasEnt, str_hash("maps/000.map"))) {
    m_pak.unmapEntry(m_atlasEnt[ATLAS_GLOBAL]);
    m_i.mem.free(m_state);
    throw log_except("Cannot load map!");
  }

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
  const uptr fileSize =
    sizeof(map_file_t)-sizeof(map_file_cube_t) +
    state.curMap->cubeCount*sizeof(map_file_cube_t);

  out.write(zeros, fileSize);
  file_mapping_t *outMap = out.map(FILE_MAP_READWRITE, 0, fileSize);
  if (!outMap) throw log_except("Couldn't map output map file!");

  map_file_t *map = (map_file_t*)outMap->data;

  map->magic = MAP_MAGIC;
  map->cubeCount = state.curMap->cubeCount;
  map->prevLoad.min.v() = state.curMap->prevLoad.min;
  map->prevLoad.max.v() = state.curMap->prevLoad.max;
  map->nextLoad.min.v() = state.curMap->nextLoad.min;
  map->nextLoad.max.v() = state.curMap->nextLoad.max;
  map->prevLoad.map = state.curMap->prop->prev;
  map->nextLoad.map = state.curMap->prop->next;

  for (uptr i = 0; i < state.curMap->cubeCount; ++i) {
    map->cubes[i].min = state.curMap->cubes[i].min;
    map->cubes[i].max = state.curMap->cubes[i].max;
    map->cubes[i].img = state.curMap->cubes[i].img;
  }

  map->levelAtlas = state.curMap->prop->atlas;

  outMap->unmap();
}

game_update_ret_t game_t::update() {
	if (m_i.input.k.pressed[KEYC_ESCAPE]) return GAME_UPDATE_CLOSE;

  // Change map
  if (m_i.input.k.pressed[KEYC_RIGHT] && (m_state->curMap < m_state->maps+GAME_STATE_MAPCOUNT-1)) {
    ++m_state->curMap;
    if (!loadMap(m_i.mem, m_pak, *m_state, m_atlasEnt, m_state->curMap->prop->hashName))
      throw log_except("Cannot load map into editor!");
  } else if (m_i.input.k.pressed[KEYC_LEFT] && (m_state->curMap > m_state->maps)) {
    --m_state->curMap;
    if (!loadMap(m_i.mem, m_pak, *m_state, m_atlasEnt, m_state->curMap->prop->hashName))
      throw log_except("Cannot load map into editor!");
  }

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

  // Switch cube texture
  if (m_i.input.k.pressed[KEYC_Q]) {
    if (m_state->curMap->tex == 0) m_state->curMap->tex = m_state->curMap->prop->imgCount-1;
    else --m_state->curMap->tex;
  } else if (m_i.input.k.pressed[KEYC_E]) {
    if (m_state->curMap->tex == m_state->curMap->prop->imgCount-1) m_state->curMap->tex = 0;
    else ++m_state->curMap->tex;
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

  // Add & remove cubes
  if (m_i.input.k.pressed[KEYC_M_PRIMARY] && (m_state->curMap->cubeCount < 255)) ++m_state->curMap->cubeCount;
  else if (m_i.input.k.pressed[KEYC_M_SECONDARY] && (m_state->curMap->cubeCount > 0)) {
    // Remove cube at cubePos
    for (uptr i = 0; i < m_state->curMap->cubeCount; ++i) {
      if ((m_state->curMap->cubes[i].min.f[0] == cubePos.f[0]) &&
          (m_state->curMap->cubes[i].min.f[1] == cubePos.f[1]) &&
          (m_state->curMap->cubes[i].min.f[2] == cubePos.f[2]))
      {
        memcpy((void*)(m_state->curMap->cubes+i),
               m_state->curMap->cubes+i+1,
               (--m_state->curMap->cubeCount-i)*sizeof(map_cube_t));
        break;
      }
    }
  }

  // Add loading zones
  if (m_i.input.k.pressed[KEYC_INSERT]) {
    m_state->curMap->prevLoad.min = cubePos;
    m_state->curMap->prevLoad.max =
      cubePos + m_state->blockSize*m_state->blockGrid;
  } else if (m_i.input.k.pressed[KEYC_DELETE]) {
    m_state->curMap->nextLoad.min = cubePos;
    m_state->curMap->nextLoad.max =
      cubePos + m_state->blockSize*m_state->blockGrid;
  }

  m_state->curMap->cubes[m_state->curMap->cubeCount].min = cubePos;
  m_state->curMap->cubes[m_state->curMap->cubeCount].max =
    cubePos + m_state->blockSize*m_state->blockGrid;
  m_state->curMap->cubes[m_state->curMap->cubeCount].img = m_state->curMap->prop->imgNames[m_state->curMap->tex];

  // Write map
  if (m_i.input.k.pressed[KEYC_F2]) {
    file_handle_t *out = m_i.fileSys.open(m_state->curMap->prop->filename, FILE_MODE_READWRITE);
    if (out) writeMap(*out, *m_state);
    out->close();
  }

	return GAME_UPDATE_CONTINUE;
}

#endif // GAME_STATE_EDITOR
