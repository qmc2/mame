-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

defines {
	"VNC_OSD_VERSION=0.2",
	"OSD_VNC",
	"OSD_SDL",
	"USE_QTDEBUG=0",
	"SDLMAME_NOASM=1",
	"SDLMAME_UNIX",
	"USE_XINPUT=0",
}

if _OPTIONS["BIGENDIAN"]=="1" then
	defines {
		"VNC_OSD_BIGENDIAN",
	}
end

_OPTIONS["USE_QTDEBUG"] = "0"
