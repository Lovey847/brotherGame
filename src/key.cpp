#include "types.h"
#include "key.h"
#include "log.h"

#include <cstring>

#define LOG_PRESSED_RELEASED 0

key_state_t::key_state_t() {
	memset(curDown, 0, sizeof(curDown));
	
	memset(down, 0, sizeof(down));
	memset(pressed, 0, sizeof(pressed));
	memset(released, 0, sizeof(released));
}

// Press KEY_CTRL, KEY_ALT, etc. when one of the variants are pressed
#define PRESSTREE(_key)											\
	case KEYC_L ## _key: case KEYC_R ## _key:					\
    	pressKey(KEYC_ ## _key, curDown, down, pressed); break

// Release KEY_CTRL, KEY_ALT, etc. when both of the variants are released
#define RELEASETREE(_key)				    								\
	case KEYC_L ## _key:					    							\
    	if (curDown[KEYC_R ## _key]) break;									\
    	releaseKey(KEYC_ ## _key, pressed, down, released, curDown); break; \
		    																\
	case KEYC_R ## _key:			    									\
	    if (curDown[KEYC_L ## _key]) break;									\
	    releaseKey(KEYC_ ## _key, pressed, down, released, curDown); break

static FINLINE void pressKey(key_code_t key, ubool *curDown, ubool *down, ubool *pressed) {
	down[key] = true;
	curDown[key] = true;
	pressed[key] = true;
}

static FINLINE void releaseKey(key_code_t key, ubool *pressed, ubool *down, ubool *released, ubool *curDown) {
	// I only want down[key] to wait a frame to become false if it was also pressed in the
	// same frame, if it wasn't pressed this frame it should become false immediately
	if (!pressed[key]) down[key] = false;
	curDown[key] = false;
	released[key] = true;
}

void key_state_t::press(key_code_t key) {
	log_assert(key != KEYC_SHIFT, "KEYC_SHIFT should never be used in press!");
	log_assert(key != KEYC_CTRL, "KEYC_CTRL should never be used in press!");
	log_assert(key != KEYC_ALT, "KEYC_ALT should never be used in press!");
	log_assert(key != KEYC_NONE, "KEYC_NONE should never be used in press!");

	if (pressed[key]) return;

#if LOG_PRESSED_RELEASED
	log_note("Key %d pressed", (int)key);
#endif
	
	pressKey(key, curDown, down, pressed);
	
	switch (key) {
	PRESSTREE(SHIFT);
	PRESSTREE(CTRL);
	PRESSTREE(ALT);
	
	default: ;
	}
}

void key_state_t::release(key_code_t key) {
	log_assert(key != KEYC_SHIFT, "KEYC_SHIFT should never be used in release!");
	log_assert(key != KEYC_CTRL, "KEYC_CTRL should never be used in release!");
	log_assert(key != KEYC_ALT, "KEYC_ALT should never be used in release!");
	log_assert(key != KEYC_NONE, "KEYC_NONE should never be used in release!");

	if (released[key]) return;

#if LOG_PRESSED_RELEASED
	log_note("Key %d released", (int)key);
#endif
	
	releaseKey(key, pressed, down, released, curDown);
	
	switch (key) {
	RELEASETREE(SHIFT);
	RELEASETREE(CTRL);
	RELEASETREE(ALT);
	
	default: ;
	}
}

void key_state_t::update() {
	memcpy(down, curDown, sizeof(down));
	
	memset(pressed, 0, sizeof(pressed));
	memset(released, 0, sizeof(released));
}
