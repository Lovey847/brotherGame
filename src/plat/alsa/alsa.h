#ifndef ALSA_H
#define ALSA_H

#include "types.h"
#include "audio.h"
#include "log.h"
#include "module.h"
#include "mem.h"
#include "rng.h"

#include <alsa/asoundlib.h>
#include <pthread.h>

// Audio thread state
enum alsa_threadState_t : ufast {
	ALSA_THREAD_INACTIVE = 0, // The thread hasn't started yet
	ALSA_THREAD_RUNNING,      // The thread is currently running
	ALSA_THREAD_FAILED,       // An irrecoverable error has occurred in the thread
	                          // Error stored in err()
	ALSA_THREAD_SUCCESS       // The thread has shut down successfully
};

struct alsa_soundState_t {
	u32 unused;
};

// Data used by the audio thread
struct alsa_threadData_t {
	// Sound state, filled out by main thread
	// You msut own the mutex to read/write this variable
	volatile alsa_soundState_t snd;
	
	// The audio buffer, filled with sound state and written to pcm
	// by audio thread
	// This buffer is initialized and freed by the main thread,
	// but during runtime the audio thread has complete ownership over it
	audio_frame_t *buf;
	
	// Sample rate
	// Constant value from initialization
	uptr sampleRate;

	// Data mutex
	pthread_mutex_t m;

	// Destruction condition
	pthread_cond_t destruct;

	// Audio thread ID
	// Constant value from initialization
	pthread_t tid;

	// Thread state
	// You must own the mutex to read/write this variable
	volatile alsa_threadState_t state;

	// Set by main thread to close audio thread
	// You must own the mutex to read/write this variable
	volatile ubool quit;

	FINLINE log_except_t &err() {return *(log_except_t*)buf;}
	FINLINE const log_except_t &err() const {return *(log_except_t*)buf;}

	// Lock and unlock mutex for read/write access to some areas of struct
	FINLINE void lock() {pthread_mutex_lock(&m);}
	FINLINE void unlock() {pthread_mutex_unlock(&m);}

	// Waiting on destruction condition
	FINLINE void waitForDestruct() {pthread_cond_wait(&destruct, &m);}
	FINLINE void signalDestruct() {
		pthread_mutex_unlock(&m);
		pthread_cond_signal(&destruct);
	}
};

// Derived module class for ALSA API
class alsa_audio_t : public audio_base_t {
private:
	// Index used for square wave (TEST)
	f32 m_ind = 0.f;

	mem_t &m_m; // Memory interface
	alsa_threadData_t m_data; // Thread data
	const game_state_t &m_g; // Game state

public:
	alsa_audio_t(audio_init_t &init);
	~alsa_audio_t();

	ubool update();
};

#endif //ALSA_H
