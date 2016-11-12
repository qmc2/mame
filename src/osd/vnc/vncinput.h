// license:BSD-3-Clause
// copyright-holders:MAMEdev Team

#ifndef VNCINPUT_H
#define VNCINPUT_H

extern "C" {
#include <rfb/rfb.h>
#include <rfb/keysym.h>
}

extern void vnc_input_init_keyboard(input_device *);
extern void vnc_input_init_mouse(input_device *);
extern void vnc_input_set_key_state(rfbBool, rfbKeySym);
extern void vnc_input_set_mouse_state(int, int, int);
extern void vnc_input_update_mouse_movement();
extern int32_t vnc_input_get_key_state(void *, void *);
extern int32_t vnc_input_get_mouse_axis(void *, void *);
extern int32_t vnc_input_get_mouse_button(void *, void *);

#endif
