// license:BSD-3-Clause
// copyright-holders:MAMEdev Team

#include "emu.h"
#include "inputdev.h"
#include "osdepend.h"
extern "C" {
#include <rfb/rfb.h>
#include <rfb/keysym.h>
}
#include <stdint.h>

#define VNC_OSD_MAX_MOUSE_BUTTONS	8
#define VNC_OSD_KEY_STATE_PRESSED	0x80
#define VNC_OSD_KEY_STATE_RELEASED	0x00
#define VNC_OSD_KEY_STATE(p)		(p ? VNC_OSD_KEY_STATE_PRESSED : VNC_OSD_KEY_STATE_RELEASED)
#define VNC_OSD_GEN_ITEM_ID		input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_OTHER_SWITCH).item_id()

//extern bool vncOsdOverlayUiActive;
extern int32_t rfbFrameBufferWidth;
extern int32_t rfbFrameBufferHeight;

static int32_t old_mouse_x = 0;
static int32_t old_mouse_y = 0;
static int32_t deltaX = 0;
static int32_t deltaY = 0;

enum vnc_osd_keys {
	// start of keys
	VNC_OSD_KEY_INVALID,

	// VNC OSD specific keys
	VNC_OSD_KEY_EXCLAM,		// !
	VNC_OSD_KEY_SECTION,		// §
	VNC_OSD_KEY_DOLLAR,		// $
	VNC_OSD_KEY_PERCENT,		// %
	VNC_OSD_KEY_AMPERSAND,		// &
	VNC_OSD_KEY_QUESTION,		// ?

	// standard keys available in core MAME/MESS
	VNC_OSD_KEY_ESCAPE,
	VNC_OSD_KEY_TAB,
	VNC_OSD_KEY_1,
	VNC_OSD_KEY_2,
	VNC_OSD_KEY_3,
	VNC_OSD_KEY_4,
	VNC_OSD_KEY_5,
	VNC_OSD_KEY_6,
	VNC_OSD_KEY_7,
	VNC_OSD_KEY_8,
	VNC_OSD_KEY_9,
	VNC_OSD_KEY_0,
	VNC_OSD_KEY_LEFT,
	VNC_OSD_KEY_RIGHT,
	VNC_OSD_KEY_UP,
	VNC_OSD_KEY_DOWN,
	VNC_OSD_KEY_PGUP,
	VNC_OSD_KEY_PGDN,
	VNC_OSD_KEY_ENTER,
	VNC_OSD_KEY_PAUSE,
	VNC_OSD_KEY_LSHIFT,
	VNC_OSD_KEY_RSHIFT,
	VNC_OSD_KEY_LCONTROL,
	VNC_OSD_KEY_RCONTROL,
	VNC_OSD_KEY_LALT,
	VNC_OSD_KEY_RALT,
	VNC_OSD_KEY_SCRLOCK,
	VNC_OSD_KEY_SPACE,
	VNC_OSD_KEY_BACKSPACE,
	VNC_OSD_KEY_DELETE,
	VNC_OSD_KEY_F1,
	VNC_OSD_KEY_F2,
	VNC_OSD_KEY_F3,
	VNC_OSD_KEY_F4,
	VNC_OSD_KEY_F5,
	VNC_OSD_KEY_F6,
	VNC_OSD_KEY_F7,
	VNC_OSD_KEY_F8,
	VNC_OSD_KEY_F9,
	VNC_OSD_KEY_F10,
	VNC_OSD_KEY_F11,
	VNC_OSD_KEY_F12,
	VNC_OSD_KEY_F13,
	VNC_OSD_KEY_F14,
	VNC_OSD_KEY_F15,
	VNC_OSD_KEY_A,
	VNC_OSD_KEY_B,
	VNC_OSD_KEY_C,
	VNC_OSD_KEY_D,
	VNC_OSD_KEY_E,
	VNC_OSD_KEY_F,
	VNC_OSD_KEY_G,
	VNC_OSD_KEY_H,
	VNC_OSD_KEY_I,
	VNC_OSD_KEY_J,
	VNC_OSD_KEY_K,
	VNC_OSD_KEY_L,
	VNC_OSD_KEY_M,
	VNC_OSD_KEY_N,
	VNC_OSD_KEY_O,
	VNC_OSD_KEY_P,
	VNC_OSD_KEY_Q,
	VNC_OSD_KEY_R,
	VNC_OSD_KEY_S,
	VNC_OSD_KEY_T,
	VNC_OSD_KEY_U,
	VNC_OSD_KEY_V,
	VNC_OSD_KEY_W,
	VNC_OSD_KEY_X,
	VNC_OSD_KEY_Y,
	VNC_OSD_KEY_Z,
	VNC_OSD_KEY_COLON,
	VNC_OSD_KEY_QUOTE,
	VNC_OSD_KEY_TILDE,
	VNC_OSD_KEY_SLASH,
	VNC_OSD_KEY_BACKSLASH,
	VNC_OSD_KEY_ASTERISK,
	VNC_OSD_KEY_CAPSLOCK,
	VNC_OSD_KEY_NUMLOCK,
	VNC_OSD_KEY_HOME,
	VNC_OSD_KEY_END,
	VNC_OSD_KEY_PRTSCR,
	VNC_OSD_KEY_INSERT,
	VNC_OSD_KEY_OPENBRACE,
	VNC_OSD_KEY_CLOSEBRACE,
	VNC_OSD_KEY_COMMA,
	VNC_OSD_KEY_PERIOD,
	VNC_OSD_KEY_MINUS,
	VNC_OSD_KEY_PLUS,
	VNC_OSD_KEY_EQUALS,

	// end of keys
	VNC_OSD_KEY_TOTAL
};

// input device states
static int32_t keyboard_state[VNC_OSD_KEY_TOTAL];

#define VNC_OSD_ALT_MODIFIER		(keyboard_state[VNC_OSD_KEY_LALT] == VNC_OSD_KEY_STATE_PRESSED || keyboard_state[VNC_OSD_KEY_RALT] == VNC_OSD_KEY_STATE_PRESSED)
#define VNC_OSD_SHIFT_MODIFIER		(keyboard_state[VNC_OSD_KEY_LSHIFT] == VNC_OSD_KEY_STATE_PRESSED || keyboard_state[VNC_OSD_KEY_RSHIFT] == VNC_OSD_KEY_STATE_PRESSED)
#define VNC_OSD_CONTROL_MODIFIER	(keyboard_state[VNC_OSD_KEY_LCONTROL] == VNC_OSD_KEY_STATE_PRESSED || keyboard_state[VNC_OSD_KEY_RCONTROL] == VNC_OSD_KEY_STATE_PRESSED)

#define VNC_OSD_IS_ALT_MODIFIER(k)	(k == XK_Alt_L || k == XK_Alt_R)
#define VNC_OSD_IS_SHIFT_MODIFIER(k)	(k == XK_Shift_L || k == XK_Shift_R)
#define VNC_OSD_IS_CONTROL_MODIFIER(k)	(k == XK_Control_L || k == XK_Control_R)

struct mouseState {
	int32_t x, y, buttons[VNC_OSD_MAX_MOUSE_BUTTONS + 1];
};

static struct mouseState mouse_state;

static int32_t vnc_input_get_key_state(void *device_internal, void *item_internal)
{
	int32_t *keystate = (int32_t *)item_internal;
	return *keystate;
}

void vnc_input_init_keyboard(input_device *kb)
{
	kb->add_item("Esc", ITEM_ID_ESC, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_ESCAPE]);
	kb->add_item("Tab", ITEM_ID_TAB, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_TAB]);
	kb->add_item("Space", ITEM_ID_SPACE, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_SPACE]);
	kb->add_item("BackSpace", ITEM_ID_BACKSPACE, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_BACKSPACE]);
	kb->add_item("Delete", ITEM_ID_DEL, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_DELETE]);
	kb->add_item("Insert", ITEM_ID_INSERT, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_INSERT]);
	kb->add_item("CapsLock", ITEM_ID_CAPSLOCK, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_CAPSLOCK]);
	kb->add_item("NumLock", ITEM_ID_NUMLOCK, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_NUMLOCK]);
	kb->add_item("Home", ITEM_ID_HOME, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_HOME]);
	kb->add_item("End", ITEM_ID_END, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_END]);
	kb->add_item("PrintScreen", ITEM_ID_PRTSCR, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_PRTSCR]);
	kb->add_item("Left", ITEM_ID_LEFT, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_LEFT]);
	kb->add_item("Right", ITEM_ID_RIGHT, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_RIGHT]);
	kb->add_item("Up", ITEM_ID_UP, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_UP]);
	kb->add_item("Down", ITEM_ID_DOWN, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_DOWN]);
	kb->add_item("PageUp", ITEM_ID_PGUP, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_PGUP]);
	kb->add_item("PageDown", ITEM_ID_PGDN, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_PGDN]);
	kb->add_item("Enter", ITEM_ID_ENTER, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_ENTER]);
	kb->add_item("Pause", ITEM_ID_PAUSE, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_PAUSE]);
	kb->add_item("LeftShift", ITEM_ID_LSHIFT, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_LSHIFT]);
	kb->add_item("RightShift", ITEM_ID_RSHIFT, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_RSHIFT]);
	kb->add_item("LeftControl", ITEM_ID_LCONTROL, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_LCONTROL]);
	kb->add_item("RightControl", ITEM_ID_RCONTROL, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_RCONTROL]);
	kb->add_item("LeftAlt", ITEM_ID_LALT, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_LALT]);
	kb->add_item("RightAlt", ITEM_ID_RALT, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_RALT]);
	kb->add_item("ScollLock", ITEM_ID_SCRLOCK, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_SCRLOCK]);
	kb->add_item("F1", ITEM_ID_F1, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F1]);
	kb->add_item("F2", ITEM_ID_F2, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F2]);
	kb->add_item("F3", ITEM_ID_F3, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F3]);
	kb->add_item("F4", ITEM_ID_F4, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F4]);
	kb->add_item("F5", ITEM_ID_F5, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F5]);
	kb->add_item("F6", ITEM_ID_F6, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F6]);
	kb->add_item("F7", ITEM_ID_F7, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F7]);
	kb->add_item("F8", ITEM_ID_F8, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F8]);
	kb->add_item("F9", ITEM_ID_F9, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F9]);
	kb->add_item("F10", ITEM_ID_F10, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F10]);
	kb->add_item("F11", ITEM_ID_F11, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F11]);
	kb->add_item("F12", ITEM_ID_F12, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F12]);
	kb->add_item("F13", ITEM_ID_F13, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F13]);
	kb->add_item("F14", ITEM_ID_F14, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F14]);
	kb->add_item("F15", ITEM_ID_F15, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F15]);
	kb->add_item("1", ITEM_ID_1, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_1]);
	kb->add_item("2", ITEM_ID_2, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_2]);
	kb->add_item("3", ITEM_ID_3, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_3]);
	kb->add_item("4", ITEM_ID_4, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_4]);
	kb->add_item("5", ITEM_ID_5, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_5]);
	kb->add_item("6", ITEM_ID_6, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_6]);
	kb->add_item("7", ITEM_ID_7, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_7]);
	kb->add_item("8", ITEM_ID_8, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_8]);
	kb->add_item("9", ITEM_ID_9, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_9]);
	kb->add_item("0", ITEM_ID_0, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_0]);
	kb->add_item("A", ITEM_ID_A, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_A]);
	kb->add_item("B", ITEM_ID_B, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_B]);
	kb->add_item("C", ITEM_ID_C, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_C]);
	kb->add_item("D", ITEM_ID_D, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_D]);
	kb->add_item("E", ITEM_ID_E, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_E]);
	kb->add_item("F", ITEM_ID_F, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_F]);
	kb->add_item("G", ITEM_ID_G, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_G]);
	kb->add_item("H", ITEM_ID_H, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_H]);
	kb->add_item("I", ITEM_ID_I, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_I]);
	kb->add_item("J", ITEM_ID_J, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_J]);
	kb->add_item("K", ITEM_ID_K, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_K]);
	kb->add_item("L", ITEM_ID_L, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_L]);
	kb->add_item("M", ITEM_ID_M, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_M]);
	kb->add_item("N", ITEM_ID_N, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_N]);
	kb->add_item("O", ITEM_ID_O, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_O]);
	kb->add_item("P", ITEM_ID_P, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_P]);
	kb->add_item("Q", ITEM_ID_Q, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_Q]);
	kb->add_item("R", ITEM_ID_R, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_R]);
	kb->add_item("S", ITEM_ID_S, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_S]);
	kb->add_item("T", ITEM_ID_T, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_T]);
	kb->add_item("U", ITEM_ID_U, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_U]);
	kb->add_item("V", ITEM_ID_V, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_V]);
	kb->add_item("W", ITEM_ID_W, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_W]);
	kb->add_item("X", ITEM_ID_X, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_X]);
	kb->add_item("Y", ITEM_ID_Y, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_Y]);
	kb->add_item("Z", ITEM_ID_Z, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_Z]);
	kb->add_item("Exclamation", VNC_OSD_GEN_ITEM_ID, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_EXCLAM]);
	kb->add_item("Quote", ITEM_ID_QUOTE, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_QUOTE]);
	kb->add_item("Section", VNC_OSD_GEN_ITEM_ID, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_SECTION]);
	kb->add_item("Dollar", VNC_OSD_GEN_ITEM_ID, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_DOLLAR]);
	kb->add_item("Persent", VNC_OSD_GEN_ITEM_ID, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_PERCENT]);
	kb->add_item("Ampersand", VNC_OSD_GEN_ITEM_ID, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_AMPERSAND]);
	kb->add_item("Slash", ITEM_ID_SLASH, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_SLASH]);
	kb->add_item("OpenBrace", ITEM_ID_OPENBRACE, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_OPENBRACE]);
	kb->add_item("CloseBrace", ITEM_ID_CLOSEBRACE, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_CLOSEBRACE]);
	kb->add_item("Equals", ITEM_ID_EQUALS, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_EQUALS]);
	kb->add_item("Question", VNC_OSD_GEN_ITEM_ID, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_QUESTION]);

	kb->add_item("Colon", ITEM_ID_COLON, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_COLON]);
	kb->add_item("Tilde", ITEM_ID_TILDE, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_TILDE]);
	kb->add_item("BackSlash", ITEM_ID_BACKSLASH, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_BACKSLASH]);
	kb->add_item("Asterisk", ITEM_ID_ASTERISK, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_ASTERISK]);
	kb->add_item("Comma", ITEM_ID_COMMA, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_COMMA]);
	kb->add_item("Dot", ITEM_ID_STOP, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_PERIOD]);
	kb->add_item("Minus", ITEM_ID_MINUS_PAD, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_MINUS]);
	kb->add_item("Plus", ITEM_ID_PLUS_PAD, vnc_input_get_key_state, &keyboard_state[VNC_OSD_KEY_PLUS]);
}

void vnc_input_set_key_state(rfbBool pressed, rfbKeySym key)
{
	/*
	if ( vncOsdOverlayUiActive )
		if ( key != XK_Home && !VNC_OSD_IS_CONTROL_MODIFIER(key) )
			return;
			*/
	switch ( key ) {
		case XK_Escape: keyboard_state[VNC_OSD_KEY_ESCAPE] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Tab: keyboard_state[VNC_OSD_KEY_TAB] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_space: keyboard_state[VNC_OSD_KEY_SPACE] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_BackSpace: keyboard_state[VNC_OSD_KEY_BACKSPACE] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Delete: keyboard_state[VNC_OSD_KEY_DELETE] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Insert: keyboard_state[VNC_OSD_KEY_INSERT] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Caps_Lock: keyboard_state[VNC_OSD_KEY_CAPSLOCK] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Num_Lock: keyboard_state[VNC_OSD_KEY_NUMLOCK] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Home:
			keyboard_state[VNC_OSD_KEY_HOME] = VNC_OSD_KEY_STATE(pressed);
			//if ( pressed && VNC_OSD_CONTROL_MODIFIER )
			//	vncOsdOverlayUiActive = !vncOsdOverlayUiActive;
			break;
		case XK_End: keyboard_state[VNC_OSD_KEY_END] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Print: keyboard_state[VNC_OSD_KEY_PRTSCR] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_1: keyboard_state[VNC_OSD_KEY_1] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_2: keyboard_state[VNC_OSD_KEY_2] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_3: keyboard_state[VNC_OSD_KEY_3] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_4: keyboard_state[VNC_OSD_KEY_4] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_5: keyboard_state[VNC_OSD_KEY_5] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_6: keyboard_state[VNC_OSD_KEY_6] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_7: keyboard_state[VNC_OSD_KEY_7] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_8: keyboard_state[VNC_OSD_KEY_8] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_9: keyboard_state[VNC_OSD_KEY_9] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_0: keyboard_state[VNC_OSD_KEY_0] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Left: keyboard_state[VNC_OSD_KEY_LEFT] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Right: keyboard_state[VNC_OSD_KEY_RIGHT] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Up: keyboard_state[VNC_OSD_KEY_UP] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Down: keyboard_state[VNC_OSD_KEY_DOWN] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Page_Up: keyboard_state[VNC_OSD_KEY_PGUP] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Page_Down: keyboard_state[VNC_OSD_KEY_PGDN] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Return: case XK_KP_Enter: keyboard_state[VNC_OSD_KEY_ENTER] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Pause: keyboard_state[VNC_OSD_KEY_PAUSE] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Shift_L: keyboard_state[VNC_OSD_KEY_LSHIFT] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Shift_R: keyboard_state[VNC_OSD_KEY_RSHIFT] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Control_L: keyboard_state[VNC_OSD_KEY_LCONTROL] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Control_R: keyboard_state[VNC_OSD_KEY_RCONTROL] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Alt_L: keyboard_state[VNC_OSD_KEY_LALT] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Alt_R: keyboard_state[VNC_OSD_KEY_RALT] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_Scroll_Lock: keyboard_state[VNC_OSD_KEY_SCRLOCK] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F1: keyboard_state[VNC_OSD_KEY_F1] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F2: keyboard_state[VNC_OSD_KEY_F2] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F3: keyboard_state[VNC_OSD_KEY_F3] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F4: keyboard_state[VNC_OSD_KEY_F4] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F5: keyboard_state[VNC_OSD_KEY_F5] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F6: keyboard_state[VNC_OSD_KEY_F6] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F7: keyboard_state[VNC_OSD_KEY_F7] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F8: keyboard_state[VNC_OSD_KEY_F8] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F9: keyboard_state[VNC_OSD_KEY_F9] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F10: keyboard_state[VNC_OSD_KEY_F10] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F11: keyboard_state[VNC_OSD_KEY_F11] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F12: keyboard_state[VNC_OSD_KEY_F12] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F13: keyboard_state[VNC_OSD_KEY_F13] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F14: keyboard_state[VNC_OSD_KEY_F14] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_F15: keyboard_state[VNC_OSD_KEY_F15] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_a: case XK_A: keyboard_state[VNC_OSD_KEY_A] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_b: case XK_B: keyboard_state[VNC_OSD_KEY_B] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_c: case XK_C: keyboard_state[VNC_OSD_KEY_C] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_d: case XK_D: keyboard_state[VNC_OSD_KEY_D] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_e: case XK_E: keyboard_state[VNC_OSD_KEY_E] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_f: case XK_F: keyboard_state[VNC_OSD_KEY_F] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_g: case XK_G: keyboard_state[VNC_OSD_KEY_G] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_h: case XK_H: keyboard_state[VNC_OSD_KEY_H] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_i: case XK_I: keyboard_state[VNC_OSD_KEY_I] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_j: case XK_J: keyboard_state[VNC_OSD_KEY_J] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_k: case XK_K: keyboard_state[VNC_OSD_KEY_K] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_l: case XK_L: keyboard_state[VNC_OSD_KEY_L] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_m: case XK_M: keyboard_state[VNC_OSD_KEY_M] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_n: case XK_N: keyboard_state[VNC_OSD_KEY_N] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_o: case XK_O: keyboard_state[VNC_OSD_KEY_O] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_p: case XK_P: keyboard_state[VNC_OSD_KEY_P] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_q: case XK_Q: keyboard_state[VNC_OSD_KEY_Q] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_r: case XK_R: keyboard_state[VNC_OSD_KEY_R] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_s: case XK_S: keyboard_state[VNC_OSD_KEY_S] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_t: case XK_T: keyboard_state[VNC_OSD_KEY_T] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_u: case XK_U: keyboard_state[VNC_OSD_KEY_U] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_v: case XK_V: keyboard_state[VNC_OSD_KEY_V] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_w: case XK_W: keyboard_state[VNC_OSD_KEY_W] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_x: case XK_X: keyboard_state[VNC_OSD_KEY_X] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_y: case XK_Y: keyboard_state[VNC_OSD_KEY_Y] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_z: case XK_Z: keyboard_state[VNC_OSD_KEY_Z] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_exclam: keyboard_state[VNC_OSD_KEY_EXCLAM] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_quotedbl: keyboard_state[VNC_OSD_KEY_QUOTE] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_section: keyboard_state[VNC_OSD_KEY_SECTION] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_dollar: keyboard_state[VNC_OSD_KEY_DOLLAR] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_percent: keyboard_state[VNC_OSD_KEY_PERCENT] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_ampersand: keyboard_state[VNC_OSD_KEY_AMPERSAND] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_slash: keyboard_state[VNC_OSD_KEY_SLASH] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_parenleft: keyboard_state[VNC_OSD_KEY_OPENBRACE] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_parenright: keyboard_state[VNC_OSD_KEY_CLOSEBRACE] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_equal: keyboard_state[VNC_OSD_KEY_EQUALS] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_question: keyboard_state[VNC_OSD_KEY_QUESTION] = VNC_OSD_KEY_STATE(pressed); break;

		case XK_colon: keyboard_state[VNC_OSD_KEY_COLON] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_asciitilde: case XK_dead_tilde: keyboard_state[VNC_OSD_KEY_TILDE] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_backslash: keyboard_state[VNC_OSD_KEY_BACKSLASH] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_asterisk: keyboard_state[VNC_OSD_KEY_ASTERISK] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_comma: keyboard_state[VNC_OSD_KEY_COMMA] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_period: keyboard_state[VNC_OSD_KEY_PERIOD] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_minus: keyboard_state[VNC_OSD_KEY_MINUS] = VNC_OSD_KEY_STATE(pressed); break;
		case XK_plus: keyboard_state[VNC_OSD_KEY_PLUS] = VNC_OSD_KEY_STATE(pressed); break;
		default: if ( pressed ) osd_printf_verbose("VNC-OSD-INFO: Key symbol 0x%x ignored\n", key); break;
	}
}

static int32_t vnc_input_get_mouse_axis(void *device_internal, void *item_internal)
{
	int32_t *axisdata = (int32_t *)item_internal;
	return *axisdata;
}

static int32_t vnc_input_get_mouse_button(void *device_internal, void *item_internal)
{
	int32_t *itemdata = (int32_t *)item_internal;
	return *itemdata;
}

void vnc_input_init_mouse(input_device *mouse)
{
	char devName[10];
	mouse_state.x = mouse_state.y = 0;
	mouse->add_item("X", ITEM_ID_XAXIS, vnc_input_get_mouse_axis, &mouse_state.x);
	mouse->add_item("Y", ITEM_ID_YAXIS, vnc_input_get_mouse_axis, &mouse_state.y);
	for (int button = 1; button < VNC_OSD_MAX_MOUSE_BUTTONS + 1; button++)
	{
		input_item_id itemid = (input_item_id) (ITEM_ID_BUTTON1 + button - 1);
		sprintf(devName, "B%d", button);
		mouse_state.buttons[button] = 0;
		mouse->add_item(devName, itemid, vnc_input_get_mouse_button, &mouse_state.buttons[button]);
	}
}

void vnc_input_set_mouse_state(int buttonMask, int x, int y)
{
	deltaX += x - old_mouse_x;
	deltaY += y - old_mouse_y;
	old_mouse_x = x;
	old_mouse_y = y;
	if ( buttonMask >= 0 )
		for (int button = 1; button < VNC_OSD_MAX_MOUSE_BUTTONS + 1; button++)
			mouse_state.buttons[button] = (buttonMask & button);
}

#define VNC_OSD_INPUT_RELATIVE_PER_PIXEL	4*INPUT_RELATIVE_PER_PIXEL;

void vnc_input_update_mouse_movement()
{
	mouse_state.x = deltaX * VNC_OSD_INPUT_RELATIVE_PER_PIXEL;
	mouse_state.y = deltaY * VNC_OSD_INPUT_RELATIVE_PER_PIXEL;
	deltaX = deltaY = 0;
}
