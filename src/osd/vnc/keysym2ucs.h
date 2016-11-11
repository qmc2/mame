// license:BSD-3-Clause
// copyright-holders:MAMEdev Team

#ifndef KEYSYM2UCS_H
#define KEYSYM2UCS_H

#include "emu.h"
extern "C" {
#include <rfb/rfb.h>
}

extern uint32_t keysym2ucs(rfbKeySym keysym);

#endif
