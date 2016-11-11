// license:BSD-3-Clause
// copyright-holders:MAMEdev Team

#ifndef KEYSYM2QT_H
#define KEYSYM2QT_H

#include "emu.h"
extern "C" {
#include <rfb/rfb.h>
}

extern int keysym2qt(rfbKeySym keysym);

#endif
