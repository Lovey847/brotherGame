#include "types.h"
#include "module.h"
#include "mem.h"
#include "endianUtil.h"
#include "str.h"
#include "audio.h"

// Backend includes
#ifdef PLAT_B_ALSA
#include "alsa.h"
#endif

// Dummy module backend is always included
#include "dummy_audio.h"

// Backend construction functions
#define ALSA_CONSTRUCT

#ifdef PLAT_B_ALSA

static ubool alsaConstruct(audio_base_t *out, audio_init_t &args) {
	log_note("Constructing audio module with alsa backend");
	
	try {
		(void)new(out) alsa_audio_t(args);
		return true;
	} catch (const log_except_t &err) {
		log_warning("Cannot initialize audio module with alsa backend: %s", err.str());
		return false;
	}
}

#undef ALSA_CONSTRUCT
#define ALSA_CONSTRUCT alsaConstruct,

#endif

static ubool dummyConstruct(audio_base_t *out, audio_init_t &args) {
	log_note("Constructing audio module with dummy backend");
	
	(void)args;
	(void)new(out) dummy_audio_t();
	return true;
}

#define DUMMY_CONSTRUCT dummyConstruct,

const module_constructProc_t<audio_base_t, audio_init_t> audio_construct[AUDIO_COUNT] = {
	// WARNING: This list is dependent on the order of audio_type_t in audio.h!
	DUMMY_CONSTRUCT
	ALSA_CONSTRUCT
};
