-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   vnc.lua
--
--   Rules for the building of VNC OSD
--
---------------------------------------------------------------------------

dofile("modules.lua")

function maintargetosdoptions(_target,_subtarget)
end

buildoptions {
	backtick(pkgconfigcmd() .. " --cflags Qt5Core"),
	backtick(pkgconfigcmd() .. " --cflags Qt5Network"),
	"-fPIC",
	backtick(pkgconfigcmd() .. " --cflags sdl2"),
	backtick(pkgconfigcmd() .. " --cflags SDL2_ttf"),
	backtick(pkgconfigcmd() .. " --cflags fontconfig"),
	backtick(pkgconfigcmd() .. " --cflags libvncserver"),
	backtick(pkgconfigcmd() .. " --cflags libavcodec"),
}

linkoptions {
	backtick(pkgconfigcmd() .. " --libs Qt5Core"),
	backtick(pkgconfigcmd() .. " --libs Qt5Network"),
	backtick(pkgconfigcmd() .. " --libs sdl2"),
	backtick(pkgconfigcmd() .. " --libs SDL2_ttf"),
	backtick(pkgconfigcmd() .. " --libs fontconfig"),
	backtick(pkgconfigcmd() .. " --libs libvncserver"),
	backtick(pkgconfigcmd() .. " --libs libavcodec"),
}

links {
	"m",
	"pthread",
	"GL",
	"X11",
	"asound",
	"util",
	"Qt5Core",
	"Qt5Network",
	"SDL2",
	"SDL2_ttf",
	"fontconfig",
	"vncserver",
	"avcodec",
	"avutil",
}

project ("qtdbg_" .. _OPTIONS["osd"])
	uuid (os.uuid("qtdbg_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

	dofile("vnc_cfg.lua")
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices", -- accessing imagedev from debugger
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/osd/modules/render",
		MAME_DIR .. "3rdparty",
	}
	removeflags {
		"SingleOutputDir",
	}

	files {
		MAME_DIR .. "src/osd/modules/debugger/debugqt.cpp",
	}

project ("osd_" .. _OPTIONS["osd"])
	uuid (os.uuid("osd_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

	removeflags {
		"SingleOutputDir",
	}
	
	dofile("vnc_cfg.lua")
	osdmodulesbuild()

	local MOC = ""
	if (os.is("windows")) then
		MOC = "moc"
	else
		MOCTST = backtick("which moc-qt5 2>/dev/null")			
		if (MOCTST=='') then
			MOCTST = backtick("which moc 2>/dev/null")
		end
		if (MOCTST=='') then
			print("Qt's Meta Object Compiler (moc) wasn't found!")
			os.exit(1)
		end	
		MOC = MOCTST
	end

	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices", -- accessing imagedev from debugger
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/osd/vnc",
		MAME_DIR .. "src/osd/sdl",
		MAME_DIR .. "src/osd/modules",
		MAME_DIR .. "src/osd/modules/render",
		MAME_DIR .. "src/frontend/mame",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/libjpeg",
		MAME_DIR .. "3rdparty/winpcap/Include",
		MAME_DIR .. "3rdparty/bgfx/include",
		MAME_DIR .. "3rdparty/bx/include",
	}

	files {
		MAME_DIR .. "src/osd/vnc/vncmain.cpp",
		MAME_DIR .. "src/osd/vnc/vncinput.cpp",
		MAME_DIR .. "src/osd/vnc/keysym2ucs.cpp",
		MAME_DIR .. "src/osd/vnc/audioserver.cpp",
		MAME_DIR .. "src/osd/vnc/audioserver.h",
		GEN_DIR  .. "src/osd/vnc/audioserver.moc.cpp",
	}

	custombuildtask {
		{ MAME_DIR .. "src/osd/vnc/audioserver.h", GEN_DIR  .. "src/osd/vnc/audioserver.moc.cpp", { }, { MOC .. "$(MOCINCPATH) $(<) -o $(@)" }},
	}

BASE_TARGETOS = "unix"
VNC_TARGETOS = "unix"
if _OPTIONS["targetos"]=="windows" then
	BASE_TARGETOS = "win32"
	VNC_TARGETOS = "win32"
elseif _OPTIONS["targetos"]=="macosx" then
	VNC_TARGETOS = "macosx"
end

project ("ocore_" .. _OPTIONS["osd"])
	uuid (os.uuid("ocore_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

	removeflags {
		"SingleOutputDir",	
	}

	dofile("vnc_cfg.lua")
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/osd/vnc",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
	}

	files {
		MAME_DIR .. "src/osd/osdcore.cpp",
		MAME_DIR .. "src/osd/osdcore.h",
		MAME_DIR .. "src/osd/strconv.cpp",
		MAME_DIR .. "src/osd/strconv.h",
		MAME_DIR .. "src/osd/osdsync.cpp",
		MAME_DIR .. "src/osd/osdsync.h",
		MAME_DIR .. "src/osd/modules/osdmodule.cpp",
		MAME_DIR .. "src/osd/modules/osdmodule.h",
		MAME_DIR .. "src/osd/modules/lib/osdlib_" .. VNC_TARGETOS .. ".cpp",
		MAME_DIR .. "src/osd/modules/lib/osdlib.h",
	}

	if BASE_TARGETOS=="unix" then
		files {
			MAME_DIR .. "src/osd/modules/file/posixdir.cpp",
			MAME_DIR .. "src/osd/modules/file/posixfile.cpp",
			MAME_DIR .. "src/osd/modules/file/posixfile.h",
			MAME_DIR .. "src/osd/modules/file/posixptty.cpp",
			MAME_DIR .. "src/osd/modules/file/posixsocket.cpp",
		}
	elseif BASE_TARGETOS=="win32" then
		includedirs {
			MAME_DIR .. "src/osd/windows",
		}
		files {
			MAME_DIR .. "src/osd/modules/file/windir.cpp",
			MAME_DIR .. "src/osd/modules/file/winfile.cpp",
			MAME_DIR .. "src/osd/modules/file/winfile.h",
			MAME_DIR .. "src/osd/modules/file/winptty.cpp",
			MAME_DIR .. "src/osd/modules/file/winsocket.cpp",
			MAME_DIR .. "src/osd/windows/winutil.cpp", -- FIXME put the necessary functions somewhere more appropriate
		}
	else
		files {
			MAME_DIR .. "src/osd/modules/file/stdfile.cpp",
		}
	end
