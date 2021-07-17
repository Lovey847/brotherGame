#ifndef LINUX_KEY_H
#define LINUX_KEY_H

#include "types.h"
#include "key.h"

// Xlib
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

// Get game keycode from keysym
static FINLINE key_code_t getCodeFromSym(KeySym k) {
	// There is a big gap between XK_grave and XK_BackSpace, so make 2 tables separating the two

	// First table (starts from 0x20)
	static const key_code_t table1[0x41] = {
		KEYC_SPACE, // XK_space
		KEYC_NONE, // XK_exclam
		KEYC_NONE, // XK_quotedbl
		KEYC_NONE, // XK_numbersign
		KEYC_NONE, // XK_dollar
		KEYC_NONE, // XK_percent
		KEYC_NONE, // XK_ampersand
		KEYC_QUOTE, // XK_apostrophe
		KEYC_NONE, // XK_parenleft
		KEYC_NONE, // XK_parenright
		KEYC_NONE, // XK_asterisk
		KEYC_NONE, // XK_plus
		KEYC_COMMA, // XK_comma
		KEYC_HYPHEN, // XK_minus
		KEYC_PERIOD, // XK_period
		KEYC_SLASH, // XK_slash
		KEYC_0, // XK_0
		KEYC_1, // XK_1
		KEYC_2, // XK_2
		KEYC_3, // XK_3
		KEYC_4, // XK_4
		KEYC_5, // XK_5
		KEYC_6, // XK_6
		KEYC_7, // XK_7
		KEYC_8, // XK_8
		KEYC_9, // XK_9
		KEYC_NONE, // XK_colon
		KEYC_SEMICOLON, // XK_semicolon
		KEYC_NONE, // XK_less
		KEYC_EQUALS, // XK_equal
		KEYC_NONE, // XK_greater
		KEYC_NONE, // XK_question
		KEYC_NONE, // XK_at
		KEYC_A, // XK_A
		KEYC_B, // XK_B
		KEYC_C, // XK_C
		KEYC_D, // XK_D
		KEYC_E, // XK_E
		KEYC_F, // XK_F
		KEYC_G, // XK_G
		KEYC_H, // XK_H
		KEYC_I, // XK_I
		KEYC_J, // XK_J
		KEYC_K, // XK_K
		KEYC_L, // XK_L
		KEYC_M, // XK_M
		KEYC_N, // XK_N
		KEYC_O, // XK_O
		KEYC_P, // XK_P
		KEYC_Q, // XK_Q
		KEYC_R, // XK_R
		KEYC_S, // XK_S
		KEYC_T, // XK_T
		KEYC_U, // XK_U
		KEYC_V, // XK_V
		KEYC_W, // XK_W
		KEYC_X, // XK_X
		KEYC_Y, // XK_Y
		KEYC_Z, // XK_Z
		KEYC_OPENBRACKET, // XK_bracketleft
		KEYC_BACKSLASH, // XK_backslash
		KEYC_CLOSEBRACKET, // XK_bracketright
		KEYC_NONE, // XK_asciicircum
		KEYC_NONE, // XK_underscore
		KEYC_GRAVE, // XK_grave
	};

	// Second table (starts from 0xff08)
	// TODO: Go through keysymdef.h and see if any keysym corresponds to an unused slot!
	static const key_code_t table2[0xf8] = {
		KEYC_BACKSPACE, // XK_BackSpace
		KEYC_TAB, // XK_Tab
		KEYC_RETURN, // XK_Linefeed
		KEYC_NONE, // XK_Clear
		KEYC_NONE, // 0xff0c
		KEYC_RETURN, // XK_Return
		KEYC_NONE, // 0xff0e
		KEYC_NONE, // 0xff0f
		KEYC_NONE, // 0xff10
		KEYC_NONE, // 0xff11
		KEYC_NONE, // 0xff12
		KEYC_PAUSEBRK, // XK_Pause (is this the correct key code?)
		KEYC_SCROLLLOCK, // XK_Scroll_Lock
		KEYC_NONE, // XK_Sys_Req
		KEYC_NONE, // 0xff16
		KEYC_NONE, // 0xff17
		KEYC_NONE, // 0xff18
		KEYC_NONE, // 0xff19
		KEYC_NONE, // 0xff1a
		KEYC_ESCAPE, // XK_Escape
		KEYC_NONE, // 0xff1c
		KEYC_NONE, // 0xff1d
		KEYC_NONE, // 0xff1e
		KEYC_NONE, // 0xff1f
		KEYC_NONE, // XK_Multi_key
		KEYC_NONE, // XK_Kanji
		KEYC_NONE, // XK_Muhenkan
		KEYC_NONE, // XK_Henkan_Mode
		KEYC_NONE, // XK_Romaji
		KEYC_NONE, // XK_Hiragana
		KEYC_NONE, // XK_Katakana
		KEYC_NONE, // XK_Hiragana_Katakana
		KEYC_NONE, // XK_Zenkaku
		KEYC_NONE, // XK_Hankaku
		KEYC_NONE, // XK_Zenkaku_Hankaku
		KEYC_NONE, // XK_Touroku
		KEYC_NONE, // XK_Massyo
		KEYC_NONE, // XK_Kana_Lock
		KEYC_NONE, // XK_Kana_Shift
		KEYC_NONE, // XK_Eisu_Shift
		KEYC_NONE, // XK_Eisu_toggle
		KEYC_NONE, // 0xff31
		KEYC_NONE, // 0xff32
		KEYC_NONE, // 0xff33
		KEYC_NONE, // 0xff34
		KEYC_NONE, // 0xff35
		KEYC_NONE, // 0xff36
		KEYC_NONE, // XK_Codeinput
		KEYC_NONE, // 0xff38
		KEYC_NONE, // 0xff39
		KEYC_NONE, // 0xff3a
		KEYC_NONE, // 0xff3b
		KEYC_NONE, // XK_SingleCandidate
		KEYC_NONE, // XK_MultipleCandidate
		KEYC_NONE, // XK_PreviousCandidate
		KEYC_NONE, // 0xff3f
		KEYC_NONE, // 0xff40
		KEYC_NONE, // 0xff41
		KEYC_NONE, // 0xff42
		KEYC_NONE, // 0xff43
		KEYC_NONE, // 0xff44
		KEYC_NONE, // 0xff45
		KEYC_NONE, // 0xff46
		KEYC_NONE, // 0xff47
		KEYC_NONE, // 0xff48
		KEYC_NONE, // 0xff49
		KEYC_NONE, // 0xff4a
		KEYC_NONE, // 0xff4b
		KEYC_NONE, // 0xff4c
		KEYC_NONE, // 0xff4d
		KEYC_NONE, // 0xff4e
		KEYC_NONE, // 0xff4f
		KEYC_HOME, // XK_Home
		KEYC_LEFT, // XK_Left
		KEYC_UP, // XK_Up
		KEYC_RIGHT, // XK_Right
		KEYC_DOWN, // XK_Down
		KEYC_PAGEUP, // XK_Page_Up
		KEYC_PAGEDOWN, // XK_Page_Down
		KEYC_END, // XK_End
		KEYC_NONE, // XK_Begin
		KEYC_NONE, // 0xff59
		KEYC_NONE, // 0xff5a
		KEYC_NONE, // 0xff5b
		KEYC_NONE, // 0xff5c
		KEYC_NONE, // 0xff5d
		KEYC_NONE, // 0xff5e
		KEYC_NONE, // 0xff5f
		KEYC_NONE, // XK_Select
		KEYC_PRINTSCR, // XK_Print (is this the right keycode?)
		KEYC_NONE, // XK_Execute
		KEYC_INSERT, // XK_Insert
		KEYC_NONE, // 0xff64
		KEYC_NONE, // XK_Undo
		KEYC_NONE, // XK_Redo
		KEYC_NONE, // XK_Menu
		KEYC_NONE, // XK_Find
		KEYC_NONE, // XK_Cancel
		KEYC_NONE, // XK_Help
		KEYC_PAUSEBRK, // XK_Break (is this the right keycode?)
		KEYC_NONE, // 0xff6c
		KEYC_NONE, // 0xff6d
		KEYC_NONE, // 0xff6e
		KEYC_NONE, // 0xff6f
		KEYC_NONE, // 0xff70
		KEYC_NONE, // 0xff71
		KEYC_NONE, // 0xff72
		KEYC_NONE, // 0xff73
		KEYC_NONE, // 0xff74
		KEYC_NONE, // 0xff75
		KEYC_NONE, // 0xff76
		KEYC_NONE, // 0xff77
		KEYC_NONE, // 0xff78
		KEYC_NONE, // 0xff79
		KEYC_NONE, // 0xff7a
		KEYC_NONE, // 0xff7b
		KEYC_NONE, // 0xff7c
		KEYC_NONE, // 0xff7d
		KEYC_NONE, // XK_Mode_switch
		KEYC_NUMLOCK, // XK_Num_Lock
		KEYC_NONE, // XK_KP_Space
		KEYC_NONE, // 0xff81
		KEYC_NONE, // 0xff82
		KEYC_NONE, // 0xff83
		KEYC_NONE, // 0xff84
		KEYC_NONE, // 0xff85
		KEYC_NONE, // 0xff86
		KEYC_NONE, // 0xff87
		KEYC_NONE, // 0xff88
		KEYC_NONE, // XK_KP_Tab
		KEYC_NONE, // 0xff8a
		KEYC_NONE, // 0xff8b
		KEYC_NONE, // oxff8c
		KEYC_NONE, // XK_KP_Enter
		KEYC_NONE, // 0xff8e
		KEYC_NONE, // 0xff8f
		KEYC_NONE, // 0xff90
		KEYC_NONE, // XK_KP_F1
		KEYC_NONE, // XK_KP_F2
		KEYC_NONE, // XK_KP_F3
		KEYC_NONE, // XK_KP_F4
		KEYC_NONE, // XK_KP_Home
		KEYC_NONE, // XK_KP_Left
		KEYC_NONE, // XK_KP_Up
		KEYC_NONE, // XK_KP_Right
		KEYC_NONE, // XK_KP_Down
		KEYC_NONE, // XK_KP_Page_Up
		KEYC_NONE, // XK_KP_Page_Down
		KEYC_NONE, // XK_KP_End
		KEYC_NONE, // XK_KP_Begin
		KEYC_NONE, // XK_KP_Insert
		KEYC_NONE, // XK_KP_Delete
		KEYC_NONE, // 0xffa0
		KEYC_NONE, // 0xffa1
		KEYC_NONE, // 0xffa2
		KEYC_NONE, // 0xffa3
		KEYC_NONE, // 0xffa4
		KEYC_NONE, // 0xffa5
		KEYC_NONE, // 0xffa6
		KEYC_NONE, // 0xffa7
		KEYC_NONE, // 0xffa8
		KEYC_NONE, // 0xffa9
		KEYC_MULTIPLY, // XK_KP_Multiply
		KEYC_ADD, // XK_KP_Add
		KEYC_NONE, // XK_KP_Separrator
		KEYC_SUBTRACT, // XK_KP_Subtract
		KEYC_DECIMAL, // XK_KP_Decimal
		KEYC_DIVIDE, // XK_KP_Divide
		KEYC_NUM0, // XK_KP_0
		KEYC_NUM1, // XK_KP_1
		KEYC_NUM2, // XK_KP_2
		KEYC_NUM3, // XK_KP_3
		KEYC_NUM4, // XK_KP_4
		KEYC_NUM5, // XK_KP_5
		KEYC_NUM6, // XK_KP_6
		KEYC_NUM7, // XK_KP_7
		KEYC_NUM8, // XK_KP_8
		KEYC_NUM9, // XK_KP_9
		KEYC_NONE, // 0xffba
		KEYC_NONE, // 0xffbb
		KEYC_NONE, // 0xffbc
		KEYC_NONE, // XK_KP_Equal
		KEYC_F1, // XK_F1
		KEYC_F2, // XK_F2
		KEYC_F3, // XK_F3
		KEYC_F4, // XK_F4
		KEYC_F5, // XK_F5
		KEYC_F6, // XK_F6
		KEYC_F7, // XK_F7
		KEYC_F8, // XK_F8
		KEYC_F9, // XK_F9
		KEYC_F10, // XK_F10
		KEYC_F11, // XK_F11
		KEYC_F12, // XK_F12
		KEYC_F13, // XK_F13
		KEYC_F14, // XK_F14
		KEYC_F15, // XK_F15
		KEYC_F16, // XK_F16
		KEYC_F17, // XK_F17
		KEYC_F18, // XK_F18
		KEYC_F19, // XK_F19
		KEYC_F20, // XK_F20
		KEYC_F21, // XK_F21
		KEYC_F22, // XK_F22
		KEYC_F23, // XK_F23
		KEYC_F24, // XK_F24
		KEYC_NONE, // XK_F25
		KEYC_NONE, // XK_F26
		KEYC_NONE, // XK_F27
		KEYC_NONE, // XK_F28
		KEYC_NONE, // XK_F29
		KEYC_NONE, // XK_F30
		KEYC_NONE, // XK_F31
		KEYC_NONE, // XK_F32
		KEYC_NONE, // XK_F33
		KEYC_NONE, // XK_F34
		KEYC_NONE, // XK_F35
		KEYC_LSHIFT, // XK_Shift_L
		KEYC_RSHIFT, // XK_Shift_R
		KEYC_LCTRL, // XK_Control_L
		KEYC_RCTRL, // XK_Control_R
		KEYC_CAPSLOCK, // XK_Caps_Lock
		KEYC_NONE, // XK_Shift_Lock
		KEYC_NONE, // XK_Meta_L
		KEYC_NONE, // XK_Meta_R
		KEYC_LALT, // XK_Alt_L
		KEYC_RALT, // XK_Alt_R
		KEYC_NONE, // XK_Super_L
		KEYC_NONE, // XK_Super_R
		KEYC_NONE, // XK_Hyper_L
		KEYC_NONE, // XK_Hyper_R
		KEYC_NONE, // 0xffef
		KEYC_NONE, // 0xfff0
		KEYC_NONE, // 0xfff1
		KEYC_NONE, // 0xfff2
		KEYC_NONE, // 0xfff3
		KEYC_NONE, // 0xfff4
		KEYC_NONE, // 0xfff5
		KEYC_NONE, // 0xfff6
		KEYC_NONE, // 0xfff7
		KEYC_NONE, // 0xfff8
		KEYC_NONE, // 0xfff9
		KEYC_NONE, // 0xfffa
		KEYC_NONE, // 0xfffb
		KEYC_NONE, // 0xfffc
		KEYC_NONE, // 0xfffd
		KEYC_NONE, // 0xfffe
		KEYC_DELETE, // XK_Delete
	};

	// Is key in the bigger region?
	if ((uptr)(k-0xff08) < sizeof(table2)) return table2[k-0xff08];

	// Is key in the smaller (ASCII) region?
	else if ((uptr)(k -= 32) < sizeof(table1)) return table1[k];

	// Key is in neither region
	else return KEYC_NONE;

#if 0 // Old approach
	// TODO: Look over keysymdef.h and skim through the available keycodes, to see
	//       if any of them could correspond to something other than KEYC_NONE
	switch (k) {
	case XK_BackSpace: return KEYC_BACKSPACE;
	case XK_Escape: return KEYC_ESCAPE;
	case XK_Tab: return KEYC_TAB;
	case XK_Linefeed:
	case XK_Return: return KEYC_RETURN;

	case XK_Caps_Lock: return KEYC_CAPSLOCK;
	case XK_Num_Lock: return KEYC_NUMLOCK;
	case XK_Scroll_Lock: return KEYC_SCROLLLOCK;
		
	case XK_Home: return KEYC_HOME;
	case XK_End: return KEYC_END;
	case XK_Insert: return KEYC_INSERT;
	case XK_Delete: return KEYC_DELETE;
	case XK_Page_Up: return KEYC_PAGEUP;
	case XK_Page_Down: return KEYC_PAGEDOWN;

    // TODO: Make sure these correspond to KEYC_PRINTSCR and KEYC_PAUSEBRK
	case XK_Print: return KEYC_PRINTSCR;
	case XK_Break: return KEYC_PAUSEBRK;

	case XK_Left: return KEYC_LEFT;
	case XK_Right: return KEYC_RIGHT;
	case XK_Up: return KEYC_UP;
	case XK_Down: return KEYC_DOWN;

	case XK_KP_0: return KEYC_NUM0;
	case XK_KP_1: return KEYC_NUM1;
	case XK_KP_2: return KEYC_NUM2;
	case XK_KP_3: return KEYC_NUM3;
	case XK_KP_4: return KEYC_NUM4;
	case XK_KP_5: return KEYC_NUM5;
	case XK_KP_6: return KEYC_NUM6;
	case XK_KP_7: return KEYC_NUM7;
	case XK_KP_8: return KEYC_NUM8;
	case XK_KP_9: return KEYC_NUM9;

	case XK_space: return KEYC_SPACE;

	case XK_KP_Divide: return KEYC_DIVIDE;
	case XK_KP_Add: return KEYC_ADD;
	case XK_KP_Subtract: return KEYC_SUBTRACT;
	case XK_KP_Multiply: return KEYC_MULTIPLY;
	case XK_KP_Decimal: return KEYC_DECIMAL;

	case XK_equal: return KEYC_EQUALS;
	case XK_minus: return KEYC_HYPHEN;
	case XK_comma: return KEYC_COMMA;
	case XK_period: return KEYC_PERIOD;

	case XK_0: return KEYC_0;
	case XK_1: return KEYC_1;
	case XK_2: return KEYC_2;
	case XK_3: return KEYC_3;
	case XK_4: return KEYC_4;
	case XK_5: return KEYC_5;
	case XK_6: return KEYC_6;
	case XK_7: return KEYC_7;
	case XK_8: return KEYC_8;
	case XK_9: return KEYC_9;

	case XK_slash: return KEYC_SLASH;
	case XK_semicolon: return KEYC_SEMICOLON;
	case XK_apostrophe: return KEYC_QUOTE;
	case XK_grave: return KEYC_GRAVE;
	case XK_bracketleft: return KEYC_OPENBRACKET;
	case XK_bracketright: return KEYC_CLOSEBRACKET;
	case XK_backslash: return KEYC_BACKSLASH;

	case XK_A: return KEYC_A;
	case XK_B: return KEYC_B;
	case XK_C: return KEYC_C;
	case XK_D: return KEYC_D;
	case XK_E: return KEYC_E;
	case XK_F: return KEYC_F;
	case XK_G: return KEYC_G;
	case XK_H: return KEYC_H;
	case XK_I: return KEYC_I;
	case XK_J: return KEYC_J;
	case XK_K: return KEYC_K;
	case XK_L: return KEYC_L;
	case XK_M: return KEYC_M;
	case XK_N: return KEYC_N;
	case XK_O: return KEYC_O;
	case XK_P: return KEYC_P;
	case XK_Q: return KEYC_Q;
	case XK_R: return KEYC_R;
	case XK_S: return KEYC_S;
	case XK_T: return KEYC_T;
	case XK_U: return KEYC_U;
	case XK_V: return KEYC_V;
	case XK_W: return KEYC_W;
	case XK_X: return KEYC_X;
	case XK_Y: return KEYC_Y;
	case XK_Z: return KEYC_Z;

	case XK_F1: return KEYC_F1;
	case XK_F2: return KEYC_F2;
	case XK_F3: return KEYC_F3;
	case XK_F4: return KEYC_F4;
	case XK_F5: return KEYC_F5;
	case XK_F6: return KEYC_F6;
	case XK_F7: return KEYC_F7;
	case XK_F8: return KEYC_F8;
	case XK_F9: return KEYC_F9;
	case XK_F10: return KEYC_F10;
	case XK_F11: return KEYC_F11;
	case XK_F12: return KEYC_F12;
	case XK_F13: return KEYC_F13;
	case XK_F14: return KEYC_F14;
	case XK_F15: return KEYC_F15;
	case XK_F16: return KEYC_F16;
	case XK_F17: return KEYC_F17;
	case XK_F18: return KEYC_F18;
	case XK_F19: return KEYC_F19;
	case XK_F20: return KEYC_F20;
	case XK_F21: return KEYC_F21;
	case XK_F22: return KEYC_F22;
	case XK_F23: return KEYC_F23;
	case XK_F24: return KEYC_F24;

	case XK_Shift_L: return KEYC_LSHIFT;
	case XK_Shift_R: return KEYC_RSHIFT;
	case XK_Control_L: return KEYC_LCTRL;
	case XK_Control_R: return KEYC_RCTRL;
	case XK_Alt_L: return KEYC_LALT;
	case XK_Alt_R: return KEYC_RALT;

	default: return KEYC_NONE;
	}
#endif // Old approach
}

#endif //LINUX_KEY_H
