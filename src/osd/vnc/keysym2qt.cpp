// license:BSD-3-Clause
// copyright-holders:MAMEdev Team

#include <Qt>
#include "emu.h"
extern "C" {
#include <rfb/rfb.h>
#include <rfb/keysym.h>
}

int keysym2qt(rfbKeySym keysym)
{
	// FIXME: incomplete (but mostly done)
	switch (keysym) {
		case XK_Up: return Qt::Key_Up;
		case XK_Down: return Qt::Key_Down;
		case XK_Left: return Qt::Key_Left;
		case XK_Right: return Qt::Key_Right;
		case XK_Escape: return Qt::Key_Escape;
		case XK_Tab: return Qt::Key_Tab;
		case XK_space: return Qt::Key_Space;
		case XK_BackSpace: return Qt::Key_Backspace;
		case XK_Delete: return Qt::Key_Delete;
		case XK_Insert: return Qt::Key_Insert;
		case XK_Caps_Lock: return Qt::Key_CapsLock;
		case XK_Num_Lock: return Qt::Key_NumLock;
		case XK_Scroll_Lock: return Qt::Key_ScrollLock;
		case XK_Home: return Qt::Key_Home;
		case XK_End: return Qt::Key_End;
		case XK_Print: return Qt::Key_Print;
		case XK_Return: return Qt::Key_Return;
		case XK_KP_Enter: return Qt::Key_Enter;
		case XK_Pause: return Qt::Key_Pause;
		case XK_Shift_L: case XK_Shift_R: return Qt::Key_Shift;
		case XK_Control_L: case XK_Control_R: return Qt::Key_Control;
		case XK_Alt_L: case XK_Alt_R: return Qt::Key_Alt;
		case XK_Meta_R: return Qt::Key_AltGr;
		case XK_Page_Down: return Qt::Key_PageDown;
		case XK_Page_Up: return Qt::Key_PageUp;
		case XK_exclam: return Qt::Key_Exclam;
		case XK_quotedbl: return Qt::Key_QuoteDbl;
		case XK_numbersign: return Qt::Key_NumberSign;
		case XK_dollar: return Qt::Key_Dollar;
		case XK_percent: return Qt::Key_Percent;
		case XK_ampersand: return Qt::Key_Ampersand;
		case XK_apostrophe: return Qt::Key_Apostrophe;
		case XK_parenleft: return Qt::Key_ParenLeft;
		case XK_parenright: return Qt::Key_ParenRight;
		case XK_asterisk: return Qt::Key_Asterisk;
		case XK_plus: return Qt::Key_Plus;
		case XK_minus: return Qt::Key_Minus;
		case XK_comma: return Qt::Key_Comma;
		case XK_period: return Qt::Key_Period;
		case XK_slash: return Qt::Key_Slash;
		case XK_backslash: return Qt::Key_Backslash;
		case XK_colon: return Qt::Key_Colon;
		case XK_semicolon: return Qt::Key_Semicolon;
		case XK_less: return Qt::Key_Less;
		case XK_equal: return Qt::Key_Equal;
		case XK_greater: return Qt::Key_Greater;
		case XK_question: return Qt::Key_Question;
		case XK_at: return Qt::Key_At;
		case XK_bracketleft: return Qt::Key_BracketLeft;
		case XK_bracketright: return Qt::Key_BracketRight;
		case XK_underscore: return Qt::Key_Underscore;
		case XK_quoteleft: return Qt::Key_QuoteLeft;
		case XK_braceleft: return Qt::Key_BraceLeft;
		case XK_braceright: return Qt::Key_BraceRight;
		case XK_dead_circumflex: return Qt::Key_AsciiCircum;
		case XK_dead_tilde: return Qt::Key_AsciiTilde;
		case XK_questiondown: return Qt::Key_questiondown;
		case XK_exclamdown: return Qt::Key_exclamdown;
		case XK_1: return Qt::Key_1;
		case XK_2: return Qt::Key_2;
		case XK_3: return Qt::Key_3;
		case XK_4: return Qt::Key_4;
		case XK_5: return Qt::Key_5;
		case XK_6: return Qt::Key_6;
		case XK_7: return Qt::Key_7;
		case XK_8: return Qt::Key_8;
		case XK_9: return Qt::Key_9;
		case XK_0: return Qt::Key_0;
		case XK_a: case XK_A: return Qt::Key_A;
		case XK_b: case XK_B: return Qt::Key_B;
		case XK_c: case XK_C: return Qt::Key_C;
		case XK_d: case XK_D: return Qt::Key_D;
		case XK_e: case XK_E: return Qt::Key_E;
		case XK_f: case XK_F: return Qt::Key_F;
		case XK_g: case XK_G: return Qt::Key_G;
		case XK_h: case XK_H: return Qt::Key_H;
		case XK_i: case XK_I: return Qt::Key_U;
		case XK_j: case XK_J: return Qt::Key_J;
		case XK_k: case XK_K: return Qt::Key_K;
		case XK_l: case XK_L: return Qt::Key_L;
		case XK_m: case XK_M: return Qt::Key_M;
		case XK_n: case XK_N: return Qt::Key_N;
		case XK_o: case XK_O: return Qt::Key_O;
		case XK_p: case XK_P: return Qt::Key_P;
		case XK_q: case XK_Q: return Qt::Key_Q;
		case XK_r: case XK_R: return Qt::Key_R;
		case XK_s: case XK_S: return Qt::Key_S;
		case XK_t: case XK_T: return Qt::Key_T;
		case XK_u: case XK_U: return Qt::Key_U;
		case XK_v: case XK_V: return Qt::Key_V;
		case XK_w: case XK_W: return Qt::Key_W;
		case XK_x: case XK_X: return Qt::Key_X;
		case XK_y: case XK_Y: return Qt::Key_Y;
		case XK_z: case XK_Z: return Qt::Key_Z;
		case XK_F1: return Qt::Key_F1;
		case XK_F2: return Qt::Key_F2;
		case XK_F3: return Qt::Key_F3;
		case XK_F4: return Qt::Key_F4;
		case XK_F5: return Qt::Key_F5;
		case XK_F6: return Qt::Key_F6;
		case XK_F7: return Qt::Key_F7;
		case XK_F8: return Qt::Key_F8;
		case XK_F9: return Qt::Key_F9;
		case XK_F10: return Qt::Key_F10;
		case XK_F11: return Qt::Key_F11;
		case XK_F12: return Qt::Key_F12;
		case XK_F13: return Qt::Key_F13;
		case XK_F14: return Qt::Key_F14;
		case XK_F15: return Qt::Key_F15;
		case XK_F16: return Qt::Key_F16;
		case XK_F17: return Qt::Key_F17;
		case XK_F18: return Qt::Key_F18;
		case XK_F19: return Qt::Key_F19;
		case XK_F20: return Qt::Key_F20;
		case XK_F21: return Qt::Key_F21;
		case XK_F22: return Qt::Key_F22;
		case XK_F23: return Qt::Key_F23;
		case XK_F24: return Qt::Key_F24;
		case XK_F25: return Qt::Key_F25;
		case XK_F26: return Qt::Key_F26;
		case XK_F27: return Qt::Key_F27;
		case XK_F28: return Qt::Key_F28;
		case XK_F29: return Qt::Key_F29;
		case XK_F30: return Qt::Key_F30;
		case XK_F31: return Qt::Key_F31;
		case XK_F32: return Qt::Key_F32;
		case XK_F33: return Qt::Key_F33;
		case XK_F34: return Qt::Key_F34;
		case XK_F35: return Qt::Key_F35;
		
		default: return -1;
	}
}
