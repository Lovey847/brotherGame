#ifndef DUMMY_AUDIO_H
#define DUMMY_AUDIO_H

// NULL audio backend
// Doesn't actually output any audio

#include "types.h"
#include "audio.h"

class dummy_audio_t : public audio_base_t {
public:
	FINLINE dummy_audio_t() {}
	~dummy_audio_t() {}

	ubool update() {return true;}
};

#endif //DUMMY_AUDIO_H
