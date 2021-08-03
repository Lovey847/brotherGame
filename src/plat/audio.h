#ifndef AUDIO_H
#define AUDIO_H

#include "types.h"
#include "module.h"
#include "mem.h"
#include "endianUtil.h"
#include "str.h"
#include "game/state.h"
#include "interfaces.h"
#include "log.h"

// Audio backend type enum
enum audio_type_t {
	// WARNING: The order of elements in this enum effect the audio_construct array,
	//          if the order of this enum is changed, change the order of that array as well!
	
	// Dummy is always present
#ifdef PLAT_B_ALSA
	AUDIO_ALSA,
#endif
	AUDIO_DUMMY,

	AUDIO_COUNT
};

struct audio_frame_t {
	// Left sample and right sample, interleaved
	i16 left, right;
};

typedef ubool (*audio_writeCallback_t)(audio_frame_t */*frames*/, uptr /*frameCount*/, void */*data*/);

// Initializer struct for audio backends
struct audio_init_t {
	modules_t &m;
	interfaces_t &i;

	const game_state_t &g;

	uptr sampleRate;

	constexpr audio_init_t(modules_t &mod, interfaces_t &inter, const game_state_t &state, uptr rate) :
		m(mod), i(inter), g(state), sampleRate(rate) {}
};

static constexpr uptr AUDIO_MAXSIZE = 256;
class audio_base_t : public module_base_t<AUDIO_MAXSIZE> {
public:
	virtual ~audio_base_t() {
		log_note("Destructing audio module");
	}

	// Update sound system
	// Returns false if an error occurred
	virtual ubool update() = 0;
};

// Audio module backend construction table
extern const module_constructProc_t<audio_base_t, audio_init_t> audio_construct[];

typedef module_inst_t<audio_base_t, audio_type_t, AUDIO_COUNT,
					  audio_init_t, AUDIO_MAXSIZE, audio_construct> audio_inst_t;

#endif //AUDIO_H
