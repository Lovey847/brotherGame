#ifndef KEY_H
#define KEY_H

#include "types.h"

enum key_code_t:ufast {
	// Keyboard keys
	KEYC_BACKSPACE		= 0x00,
	KEYC_ESCAPE			= 0x01,
	KEYC_TAB			= 0x02,
	KEYC_RETURN			= 0x03,
	
	KEYC_SHIFT			= 0x04,
	KEYC_CTRL			= 0x05,
	KEYC_ALT			= 0x06,
	
	KEYC_CAPSLOCK		= 0x07,
	KEYC_NUMLOCK		= 0x08,
	KEYC_SCROLLLOCK		= 0x09,
	
	KEYC_HOME			= 0x0A,
	KEYC_END			= 0x0B,
	KEYC_INSERT			= 0x0C,
	KEYC_DELETE			= 0x0D,
	KEYC_PAGEUP			= 0x0E,
	KEYC_PAGEDOWN		= 0x0F,
	
	KEYC_PRINTSCR		= 0x10,
	KEYC_PAUSEBRK		= 0x11,
	
	KEYC_UP				= 0x12,
	KEYC_DOWN			= 0x13,
	KEYC_LEFT			= 0x14,
	KEYC_RIGHT			= 0x15,
	
	KEYC_NUM0			= 0x16,
	KEYC_NUM1			= 0x17,
	KEYC_NUM2			= 0x18,
	KEYC_NUM3			= 0x19,
	KEYC_NUM4			= 0x1A,
	KEYC_NUM5			= 0x1B,
	KEYC_NUM6			= 0x1C,
	KEYC_NUM7			= 0x1D,
	KEYC_NUM8			= 0x1E,
	KEYC_NUM9			= 0x1F,
	
	KEYC_SPACE			= 0x20,
	
	KEYC_DIVIDE			= 0x21,
	KEYC_ADD			= 0x22,
	KEYC_SUBTRACT		= 0x23,
	KEYC_MULTIPLY		= 0x24,
	KEYC_DECIMAL		= 0x25,
	
	KEYC_EQUALS			= 0x2C,
	KEYC_HYPHEN			= 0x2D,
	KEYC_COMMA			= 0x2E,
	KEYC_PERIOD			= 0x2F,
	
	KEYC_0				= 0x30,
	KEYC_1				= 0x31,
	KEYC_2				= 0x32,
	KEYC_3				= 0x33,
	KEYC_4				= 0x34,
	KEYC_5				= 0x35,
	KEYC_6				= 0x36,
	KEYC_7				= 0x37,
	KEYC_8				= 0x38,
	KEYC_9				= 0x39,
	
	KEYC_SLASH			= 0x3A,
	KEYC_SEMICOLON		= 0x3B,
	KEYC_QUOTE			= 0x3C,
	KEYC_GRAVE			= 0x3D, // `
	KEYC_OPENBRACKET	= 0x3E,
	KEYC_CLOSEBRACKET	= 0x3F,
	KEYC_BACKSLASH		= 0x40,
	
	KEYC_A				= 0x41,
	KEYC_B				= 0x42,
	KEYC_C				= 0x43,
	KEYC_D				= 0x44,
	KEYC_E				= 0x45,
	KEYC_F				= 0x46,
	KEYC_G				= 0x47,
	KEYC_H				= 0x48,
	KEYC_I				= 0x49,
	KEYC_J				= 0x4A,
	KEYC_K				= 0x4B,
	KEYC_L				= 0x4C,
	KEYC_M				= 0x4D,
	KEYC_N				= 0x4E,
	KEYC_O				= 0x4F,
	KEYC_P				= 0x50,
	KEYC_Q				= 0x51,
	KEYC_R				= 0x52,
	KEYC_S				= 0x53,
	KEYC_T				= 0x54,
	KEYC_U				= 0x55,
	KEYC_V				= 0x56,
	KEYC_W				= 0x57,
	KEYC_X				= 0x58,
	KEYC_Y				= 0x59,
	KEYC_Z				= 0x5A,
	
	KEYC_F1				= 0x5B,
	KEYC_F2				= 0x5C,
	KEYC_F3				= 0x5D,
	KEYC_F4				= 0x5E,
	KEYC_F5				= 0x5F,
	KEYC_F6				= 0x60,
	KEYC_F7				= 0x61,
	KEYC_F8				= 0x62,
	KEYC_F9				= 0x63,
	KEYC_F10			= 0x64,
	KEYC_F11			= 0x65,
	KEYC_F12			= 0x66,
	KEYC_F13			= 0x67,
	KEYC_F14			= 0x68,
	KEYC_F15			= 0x69,
	KEYC_F16			= 0x6A,
	KEYC_F17			= 0x6B,
	KEYC_F18			= 0x6C,
	KEYC_F19			= 0x6D,
	KEYC_F20			= 0x6E,
	KEYC_F21			= 0x6F,
	KEYC_F22			= 0x70,
	KEYC_F23			= 0x71,
	KEYC_F24			= 0x72,
	
	KEYC_LCTRL		    = 0x73,
	KEYC_RCTRL		    = 0x74,
	KEYC_LSHIFT			= 0x75,
	KEYC_RSHIFT			= 0x76,
	KEYC_LALT			= 0x77,
	KEYC_RALT			= 0x78,
	
	// Mouse buttons
	KEYC_M_PRIMARY		= 0x79,
	KEYC_M_MIDDLE		= 0x7A,
	KEYC_M_SECONDARY	= 0x7B,
	
	KEYC_M_SCROLLUP		= 0x7C,
	KEYC_M_SCROLLDOWN	= 0x7D,
	
	KEYC_COUNT			= 0x7E,
	
	KEYC_NONE			= 0xFF
};

class key_state_t {
private:
	// For the feel of the game, if you press and release a key in one frame,
	// down[key] will stay true, and turn false the next frame.
	// This is done with curDown
	ubool curDown[KEYC_COUNT];
	
public:
	// If you press a key, down[key] becomes true the frame the key was pressed,
	// if you release a key, down[key] becomes false the frame the key was released
	// (except if you release it the same frame you pressed it, in which case it'll
	//  become false the next frame)
	ubool down[KEYC_COUNT];
	ubool pressed[KEYC_COUNT];
	ubool released[KEYC_COUNT];
	
	key_state_t();
	
	// ===== Functions used by platform layer to manage keyboard state =====
	
	// Modifier keys such as KEY_SHIFT, KEY_CTRL, etc. are wrappers for
	// both the variants (KEY_LSHIFT, KEY_RCTRL, etc.), and should
	// never be used in calls to press or release

	// KEYC_NONE should never be used in calls to press or release

	// Press and release silently ignore keys that were already pressed and released
	// this tick
	void press(key_code_t key);
	void release(key_code_t key);
	
	// Update keyboard state, run after game tick to update pressed and released keys
	void update();
};

#endif //KEY_H
