// TODO: Figure out what's going on with the audio and why
//       it underrun's over and over if I don't add a delay in the actual buffer!
//       Also add support for variable buffer sizes and frame sizes

#define _XOPEN_SOURCE 500

#include "types.h"
#include "audio.h"
#include "alsa.h"
#include "log.h"
#include "endianUtil.h"

#include <alsa/asoundlib.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifdef PLAT_E_LITTLE
#define SAMPLEFORMAT SND_PCM_FORMAT_S16_LE
#else
#define SAMPLEFORMAT SND_PCM_FORMAT_S16_BE
#endif

// ALSA audio device
// TODO: Should I make these configurable?
#define PCM_DEVICE "default"
#define PCM_BUFSIZE 1024

// ALSA initialization function
// Returns NULL on failure
static snd_pcm_t *tInit(alsa_threadData_t &data) {
	// Error checking macros
#define alsaCheck(_ret, ...)											\
	if ((err = (_ret)) < 0) {											\
		data.err() = log_except(__VA_ARGS__, err, snd_strerror(err));	\
		data.state = ALSA_THREAD_FAILED;								\
		data.unlock();													\
																		\
		snd_pcm_close(handle);											\
																		\
		return NULL;													\
	}

#define alsaCheckNoClose(_ret, ...)										\
	if ((err = (_ret)) < 0) {											\
		data.err() = log_except(__VA_ARGS__, err, snd_strerror(err));	\
		data.state = ALSA_THREAD_FAILED;								\
		data.unlock();													\
																		\
		return NULL;													\
	}

#define condCheck(_cond, ...)					\
	if (_cond) {								\
		data.err() = log_except(__VA_ARGS__);	\
		data.state = ALSA_THREAD_FAILED;		\
		data.unlock();							\
												\
		return NULL;							\
	}
	
	snd_pcm_t *handle = NULL;
	snd_pcm_hw_params_t *hw;
	int err;

	// Initialize PCM device
	alsaCheckNoClose(snd_pcm_open(&handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0),
					 "Cannot open " PCM_DEVICE "! (%d, %s)\n");

	// Get default hardware configuration
	snd_pcm_hw_params_alloca(&hw);
	alsaCheck(snd_pcm_hw_params_any(handle, hw),
			  "Cannot find any audio configurations! (%d, %s)\n");

	// Set channel count
	alsaCheck(snd_pcm_hw_params_set_channels(handle, hw, 2),
			  "Cannot set number of channels to 2! (%d, %s)\n");

	// Set sample rate
	alsaCheck(snd_pcm_hw_params_set_rate(handle, hw, data.sampleRate, 0),
			  "Cannot set sample rate to %d! (%d, %s)\n", data.sampleRate);

	// Set sample format
	alsaCheck(snd_pcm_hw_params_set_format(handle, hw, SAMPLEFORMAT),
			  "Cannot set sample format to 16-bit LE! (%d, %s)\n");

	// Set buffer size
	alsaCheck(snd_pcm_hw_params_set_buffer_size(handle,  hw, PCM_BUFSIZE),
			  "Cannot set buffer size to %u frames! (%d, %s)\n", PCM_BUFSIZE);

	// Set access mode
	alsaCheck(snd_pcm_hw_params_set_access(handle, hw, SND_PCM_ACCESS_RW_INTERLEAVED),
			  "Cannot set access mode to interleaved samples! (%d, %s)\n");

	// Apply hardware parameters
	alsaCheck(snd_pcm_hw_params(handle, hw),
			  "Cannot apply hardware configuration! (%d, %s)\n");

	// PCM device successfully initialized
	return handle;

#undef condCheck
#undef alsaCheck
}

// ALSA thread function
static void *tFunc(void *ptr) {
	alsa_threadData_t &data = *(alsa_threadData_t*)ptr;
	int err;

	// Free thread without required pthread_join
	pthread_detach(data.tid);

	// Initialize ALSA
	data.lock();
	snd_pcm_t *handle = tInit(data);

	if (!handle) pthread_exit(NULL); // tInit handles all error reporting

	// Start sound loop
	data.state = ALSA_THREAD_RUNNING;

	snd_pcm_prepare(handle);
	
	// Write dummy frames
	if (((err = snd_pcm_writei(handle, data.buf, PCM_BUFSIZE)) < 0) ||
		((err = snd_pcm_writei(handle, data.buf, PCM_BUFSIZE)) < 0))
	{
		data.err() = log_except("Error playing dummy samples! (%d, %s)\n", (int)err, snd_strerror(err));
		data.state = ALSA_THREAD_FAILED;
		data.unlock();

		snd_pcm_close(handle);
		pthread_exit(NULL);
	}
	
	data.unlock();

	snd_pcm_sframes_t frames;
	for (;;) {
		data.lock();
		
		if (data.quit) break;

		data.unlock();
		
		frames = snd_pcm_writei(handle, data.buf, PCM_BUFSIZE);

		if (frames == -EPIPE) {
			log_note("Underrun!");
			snd_pcm_prepare(handle);
		} else if (frames < 0) {
			// If an irrecoverable error occurred, fill transfer with error
			snd_pcm_drain(handle);

			data.lock();
			data.err() = log_except("Error playing samples! (%d, %s)\n", (int)frames, snd_strerror(frames));
			data.state = ALSA_THREAD_FAILED;

			data.unlock();

			snd_pcm_close(handle);
			pthread_exit(NULL);
		}
	}

	// Shut down
	snd_pcm_drain(handle);

	data.signalDestruct();

	snd_pcm_close(handle);

	pthread_exit(NULL);
}

// ALSA audio backend constructor, create thread
alsa_audio_t::alsa_audio_t(audio_init_t &init) :
	m_m(init.i.mem), m_g(init.g)
{
	// Initialize data
	m_data.sampleRate = init.sampleRate;

	m_data.buf = (audio_frame_t*)m_m.alloc(PCM_BUFSIZE*sizeof(audio_frame_t));

	memset(m_data.buf, 0, PCM_BUFSIZE*sizeof(audio_frame_t));

	// Create mutex
	int ret = pthread_mutex_init(&m_data.m, NULL);
	if (ret)
		throw log_except("Cannot initialize mutex! (%d, %s)", ret, strerror(ret));

	// Lock the mutex
	pthread_mutex_lock(&m_data.m);

	// Create destruct condition
	ret = pthread_cond_init(&m_data.destruct, NULL);
	if (ret)
		throw log_except("Cannot initialize destruction condition! (%d, %s)", ret, strerror(ret));
	
	m_data.state = ALSA_THREAD_INACTIVE;
	m_data.quit = false;
	
	// Create audio handling thread
	ret = pthread_create(&m_data.tid, NULL, tFunc, (void*)&m_data);
	if (ret) {
		pthread_cond_destroy(&m_data.destruct);
		pthread_mutex_unlock(&m_data.m);
		pthread_mutex_destroy(&m_data.m);
		
		throw log_except("Cannot create audio thread! (%d, %s)", ret, strerror(ret));
	}

	// Wait for thread to finish initialization
	m_data.unlock();
	usleep(1); // Give audio thread time to claim mutex
	m_data.lock();

	// If audio thread failed to initialize, error out
	if (m_data.state == ALSA_THREAD_FAILED) {
		// Don't destroy while locked
		pthread_cond_destroy(&m_data.destruct);
		pthread_mutex_unlock(&m_data.m);
		pthread_mutex_destroy(&m_data.m);

		// Free audio buffers
		m_m.free(m_data.buf);
		
		throw m_data.err();
	}

	// Thread initialization completed successfully
	m_data.unlock();
}

alsa_audio_t::~alsa_audio_t() {
	m_data.lock();
	
	// Shut down audio thread, if it's currently running
	if (m_data.state == ALSA_THREAD_RUNNING) {
		m_data.quit = true;

		// Wait for audio thread to shut down
		m_data.waitForDestruct();
	} else if (m_data.state == ALSA_THREAD_FAILED) {
		// An error occurred, report it
		log_warning(m_data.err().str());
	}

	// Free audio buffer
	m_m.free(m_data.buf);

	// Free the mutex
	pthread_cond_destroy(&m_data.destruct);
	pthread_mutex_unlock(&m_data.m);
	pthread_mutex_destroy(&m_data.m);
}

ubool alsa_audio_t::update() {
	m_data.lock();

	// If an error occurred, return false
	// The destructor will free the resources when the time comes
	if (m_data.state == ALSA_THREAD_FAILED) {
		m_data.unlock();
		
		log_warning(m_data.err().str());
		return false;
	}

	m_data.unlock();
	
	return true;
}
