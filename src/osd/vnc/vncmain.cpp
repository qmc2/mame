// license:BSD-3-Clause
// copyright-holders:MAMEdev Team

#include <QLocale>
#include <QHostInfo>

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "emu.h"
#include "osdepend.h"
#include "mame.h"
#include "clifront.h"
#include "inputdev.h"
#include "render.h"
#include "uiinput.h"
#include "keysym2ucs.h"
#include "osdvnc.h"
#include "vncinput.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

#ifdef max
#undef max // this is required for gcc 5.4 on Linux Mint 18 at least
#endif
#include "rendersw.hxx"

//============================================================
//  CONSTANTS
//============================================================

#define VNC_OSD_BITS_PER_SAMPLE		8
#define VNC_OSD_SAMPLES_PER_PIXEL	1
#define VNC_OSD_BYTES_PER_PIXEL		4
#define VNC_OSD_UPDATE_QUAD_SIZE	50	// 50x50 pixels² (or smaller at the edges)
#define VNC_OSD_MOUSE_MOVE_TIMEOUT	1000	// OSD ticks
#define VNC_OSD_REFRESH_TIMEOUT		200000	// OSD ticks
#define VNC_OSD_DOUBLECLICK_TIMEOUT	250000	// OSD ticks
#define VNC_OSD_PERFINFO_FRAMES		60	// output performance-data every 60 frames
#define VNC_OSD_ONE_KILOBYTE		1024.0f
#define VNC_OSD_ONE_MEGABYTE		1048576.0f
#define VNC_OSD_ONE_GIGABYTE		1073741824.0f

#define VNC_OSD_NO_DRIVER_LOADED	QString(m_machine->system().name).compare(m_emptyDriverName) == 0

//============================================================
//  MACROS
//============================================================

// make a string out of a non-string constant
#define VNC_OSD_STR(s)			#s
#define VNC_OSD_XSTR(s)			VNC_OSD_STR(s)

// min/max of two constants
#define VNC_OSD_MIN(a, b)		(((a) < (b)) ? (a) : (b))
#define VNC_OSD_MAX(a, b)		(((a) > (b)) ? (a) : (b))

//============================================================
//  GLOBALS
//============================================================

running_machine *vnc_osd_interface::m_machine = 0;
int32_t vnc_osd_interface::m_maxQuadsX = 0;
int32_t vnc_osd_interface::m_maxQuadsY = 0;

// a single rendering target
static render_target *vnc_render_target = 0;

// input devices
static input_device *keyboard_device = 0;
static input_device *mouse_device = 0;

// AV codec related
AVCodec *codec = 0;
AVCodecContext *codecContext = 0;
FILE *mp2File = 0;

// RFB server related
rfbScreenInfoPtr rfbScreen = 0;
char *rfbShadowFrameBuffer;
int rfbArgc;
char **rfbArgv;
int rfbClientCounter = 0;
bool rfbUseMouse = false;
bool rfbAdjustFB = true;
bool rfbAutoPause = true;
bool rfbVerbose = false;
int32_t rfbFrameBufferWidth = 0;
int32_t rfbFrameBufferHeight = 0;
int32_t rfbLastFrameBufferWidth = 0;
int32_t rfbLastFrameBufferHeight = 0;
uint32_t rfbBufferSize = 0;
uint32_t rfbScanLineSize = 0;
osd_ticks_t rfbResetPause = 0;
osd_ticks_t rfbRefreshFrameBuffer = 0;
bool rfbShadowValid = false;
bool rfbWasAutoPaused = false;
int rfbScreenCount = 0;
int rfbScale = 1;
QByteArray rfbDesktopName;

const options_entry vnc_options::vnc_option_entries[] =
{
	{ 0,			0,		OPTION_HEADER,		"VNC OPTIONS" },
	{ "vnc_port",		"5900",		OPTION_INTEGER,		"TCP port to listen on for incoming VNC connections (default: 5900)" },
	{ "vnc_adjust_fb",	"1",		OPTION_BOOLEAN,		"Auto-adjust the frame-buffer width to be a multiple of 4 for best client compatibility (default: 1)" },
	{ "vnc_autopause",	"1",		OPTION_BOOLEAN,		"Pause the machine when all clients disconnected, resume it when a client connects (default: 1)" },
	{ "vnc_mp2write",	"0",		OPTION_BOOLEAN,		"Writes MP2 encoded audio data to 'mame_audio_stream.mp2' in the current working directory (default: 0)" },
	{ "vnc_audio_bitrate",	"128000",	OPTION_INTEGER,		"Audio encoder bit rate (default: 128000)" },
	{ "vnc_audio_port",	"6900",		OPTION_INTEGER,		"Audio server UDP port (default: 6900)" },
	{ "vnc_audio_maxconn",	"32",		OPTION_INTEGER,		"Maximum number of client connections at a time (default: 32)" },

	// end of list
	{ 0 }
};

vnc_options::vnc_options()
{
	add_entries(vnc_option_entries);
}

//============================================================
//  main
//============================================================

int main(int argc, char *argv[])
{
	rfbArgc = argc;
	rfbArgv = argv;
	rfbLog = vnc_osd_interface::rfbLog;
	rfbErr = vnc_osd_interface::rfbLog;

	// instantiate options and OSD
	vnc_options options;
	vnc_osd_interface vnc_osd(options);
	vnc_osd.register_options();

	// run the actual emulation
	int returnCode = emulator_info::start_frontend(options, vnc_osd, argc, argv);
	
	// clean up
	if ( mp2File )
		fclose(mp2File);
	if ( rfbScreen ) {
		rfbScreenCleanup(rfbScreen);
		free(rfbShadowFrameBuffer);
	}

	return returnCode;
}

//============================================================
//  constructor
//============================================================

vnc_osd_interface::vnc_osd_interface(vnc_options &options) :
	osd_common_t(options),
	m_options(options),
	m_frameCounter(0),
	m_frameChangePercent(0.0),
	m_frameBufferSize(0.0),
	m_rawAudioBytes(0),
	m_encodedAudioBytes(0),
	m_encoderBufferSize(0),
	m_audioServer(0),
	m_emptyDriverName("___empty"),
	m_mewUiActive(false)
{
	// NOP
}

//============================================================
//  destructor
//============================================================

vnc_osd_interface::~vnc_osd_interface()
{
	if ( audio_server() ) {
		audio_server()->exit();
		delete audio_server();
	}
}

//============================================================
//  init
//============================================================

void vnc_osd_interface::init(running_machine &machine)
{
	// call our parent
	osd_common_t::init(machine);
	m_machine = &machine;

	// check if mewui is active and set the VNC OSD's internal property accordingly (there doesn't seem to exist an official way to ask the MAME core for it)
	setMewUiActive(VNC_OSD_NO_DRIVER_LOADED);

	// log banner
	osd_printf_verbose("VNC OSD v%s\n", VNC_OSD_XSTR(VNC_OSD_VERSION));

	vnc_options &options = downcast<vnc_options &>(machine.options());

	if ( !rfbScreen ) {
		// initialize the RFB server
		rfbVerbose = options.verbose();
		rfbLogEnable(rfbVerbose);
		rfbScreen = rfbGetScreen(&rfbArgc, rfbArgv, rfbFrameBufferWidth, rfbFrameBufferHeight, VNC_OSD_BITS_PER_SAMPLE, VNC_OSD_SAMPLES_PER_PIXEL, VNC_OSD_BYTES_PER_PIXEL);
		rfbScanLineSize = rfbFrameBufferWidth * VNC_OSD_BYTES_PER_PIXEL;
		rfbBufferSize = rfbScanLineSize * rfbFrameBufferHeight;
		rfbScreen->frameBuffer = (char *)malloc(rfbBufferSize);
		rfbShadowFrameBuffer = (char *)malloc(rfbBufferSize);
		memset(rfbScreen->frameBuffer, 0, rfbBufferSize);
		memset(rfbShadowFrameBuffer, 0, rfbBufferSize);
		rfbDesktopName = QString("%1@%2:%3").arg(emulator_info::get_appname()).arg(QHostInfo::localHostName()).arg(options.vnc_port() - 5900).toLocal8Bit();
		rfbScreen->desktopName = rfbDesktopName.constData();
		rfbScreen->alwaysShared = true;
		rfbScreen->ptrAddEvent = vnc_osd_interface::rfbProcessMouseEvent;
		rfbScreen->kbdAddEvent = vnc_osd_interface::rfbProcessKeyEvent;
		rfbScreen->newClientHook = vnc_osd_interface::rfbProcessNewClient;
		rfbScreen->port = rfbScreen->ipv6port = options.vnc_port();
		rfbScreen->deferUpdateTime = 0;
		rfbInitServer(rfbScreen);
		rfbRunEventLoop(rfbScreen, -1, true);
		rfbLastFrameBufferWidth = rfbFrameBufferWidth;
		rfbLastFrameBufferHeight = rfbFrameBufferHeight;
		rfbUseMouse = options.mouse();
		rfbAdjustFB = options.vnc_adjust_fb();
		rfbAutoPause = options.vnc_autopause();
		rfbScale = VNC_OSD_MAX(1, options.prescale());
		rfbShadowValid = false;
	}

	// initialize the video system by allocating a rendering target and setting a view
	vnc_render_target = machine.render().target_alloc();

	for (const screen_device &screen : screen_device_iterator(machine.root_device()))
		osd_printf_verbose("Screen #%d: %dx%d\n", rfbScreenCount++, screen.width(), screen.height());

	// we need to call this initially -- before setting the actual view (!) -- to get a sane state on the *configured*
	// view, otherwise machine-config saving (with regard to the layout-view) won't work correctly on machine exit
	vnc_render_target->get_primitives();

	// now configure the view (from stored config)
	QString view_name(options.view());
	if ( view_name != "auto" )
		vnc_render_target->set_view(vnc_render_target->configured_view(options.view(), 0, 1));
	else
		vnc_render_target->set_view(0); // auto => "standard"

	// initialize the input system by adding devices
	keyboard_device = machine.input().device_class(DEVICE_CLASS_KEYBOARD).add_device("Keyboard", "KBD", 0);
	if ( keyboard_device == 0 )
		fatalerror("Error creating keyboard device\n");

	// enable mouse only when requested
	if ( rfbUseMouse ) {
		mouse_device = machine.input().device_class(DEVICE_CLASS_MOUSE).add_device("Mouse", "MSE", 0);
		if ( mouse_device == 0 )
			fatalerror("Error creating mouse device\n");
	}

	// initialize the VNC keyboard & mouse devices
	vnc_input_init_keyboard(keyboard_device);
	if ( rfbUseMouse )
		vnc_input_init_mouse(mouse_device);

	// initialize audio
	init_audio();

	// get notified on emu exit
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&vnc_osd_interface::vnc_exit, &machine));

	osd_common_t::init_subsystems();
}

//============================================================
//  vnc_exit
//============================================================

void vnc_osd_interface::vnc_exit(running_machine *)
{
	rfbScreenCount = 0;
	rfbResetPause = osd_ticks();
	if ( codecContext ) {
		avcodec_close(codecContext);
		av_free(codecContext);
	}
}

//============================================================
//  osd_update
//============================================================

void vnc_osd_interface::update(bool skip_redraw)
{
	// get the minimum width/height for the current layout and make that the (scaled) size of our target
	int32_t tempwidth, tempheight;
	vnc_render_target->compute_minimum_size(tempwidth, tempheight);
	tempwidth *= rfbScale; tempheight *= rfbScale;
	float aspect;
	if ( mewUiActive() )
		aspect = tempwidth / tempheight;
	else
		aspect = vnc_render_target->pixel_aspect();
	vnc_render_target->compute_visible_area(tempwidth, tempheight, aspect, vnc_render_target->orientation(), rfbFrameBufferWidth, rfbFrameBufferHeight);
	vnc_render_target->set_bounds(rfbFrameBufferWidth, rfbFrameBufferHeight, aspect);

	// some RFB clients have problems when the frame-buffer width is not a multiple of 4
	if ( rfbAdjustFB && (rfbFrameBufferWidth & 3) )
		rfbFrameBufferWidth += 4 - (rfbFrameBufferWidth & 3);

	if ( rfbFrameBufferWidth != rfbLastFrameBufferWidth || rfbFrameBufferHeight != rfbLastFrameBufferHeight ) {
		rfbNewFrameBuffer(rfbFrameBufferWidth, rfbFrameBufferHeight);
		rfbLastFrameBufferWidth = rfbFrameBufferWidth;
		rfbLastFrameBufferHeight = rfbFrameBufferHeight;
		rfbScanLineSize = rfbFrameBufferWidth * VNC_OSD_BYTES_PER_PIXEL;
		rfbBufferSize = rfbScanLineSize * rfbFrameBufferHeight;
		m_frameBufferSize = rfbFrameBufferWidth * rfbFrameBufferHeight;
		return;
	}

	if ( rfbResetPause > 0 ) {
		if ( osd_ticks() - rfbResetPause > VNC_OSD_REFRESH_TIMEOUT )
			rfbResetPause = 0;
		else
			return;
	}

	if ( rfbRefreshFrameBuffer > 0 ) {
		if ( osd_ticks() - rfbRefreshFrameBuffer > VNC_OSD_REFRESH_TIMEOUT ) {
			rfbRefreshFrameBuffer = 0;
			rfbShadowValid = false;
		}
	}

	if ( !skip_redraw && rfbClientCounter > 0 ) {
		if ( rfbUseMouse )
			vnc_input_update_mouse_movement();

		// render the emulated screen
		render_primitive_list &primlist = vnc_render_target->get_primitives();
		primlist.acquire_lock();
#if defined(VNC_OSD_BIGENDIAN)
		software_renderer<uint32_t, 0,0,0, 16,8,0>::draw_primitives(primlist, rfbScreen->frameBuffer, rfbFrameBufferWidth, rfbFrameBufferHeight, rfbFrameBufferWidth);
#else
		software_renderer<uint32_t, 0,0,0, 0,8,16>::draw_primitives(primlist, rfbScreen->frameBuffer, rfbFrameBufferWidth, rfbFrameBufferHeight, rfbFrameBufferWidth);
#endif
		primlist.release_lock();

		QList<QRect> &modified_quads = find_modified_quads();
		if ( !modified_quads.isEmpty() ) {
			if ( rfbVerbose ) {
				foreach (QRect r, modified_quads) {
					rfbMarkRectAsModified(rfbScreen, r.topLeft().x(), r.topLeft().y(), r.bottomRight().x() + 1, r.bottomRight().y() + 1);
					m_frameChangePercent += (double)((r.width() - 1) * (r.height() - 1)) / (double)m_frameBufferSize;
				}
			} else
				foreach (QRect r, modified_quads)
					rfbMarkRectAsModified(rfbScreen, r.topLeft().x(), r.topLeft().y(), r.bottomRight().x() + 1, r.bottomRight().y() + 1);
			memcpy(rfbShadowFrameBuffer, rfbScreen->frameBuffer, rfbBufferSize);
			rfbShadowValid = true;
		}

		if ( rfbVerbose ) {
			if ( ++m_frameCounter >= VNC_OSD_PERFINFO_FRAMES ) {
				double frameTotalBytes = (double)m_frameCounter * (double)rfbBufferSize;
				m_frameChangePercent /= (double)m_frameCounter;
				osd_printf_verbose("Video RFB updates: %.1f%% [%s / %s]\n",
						   100.0 * m_frameChangePercent,
						   human_readable_value(frameTotalBytes * m_frameChangePercent).toLocal8Bit().constData(),
						   human_readable_value(frameTotalBytes).toLocal8Bit().constData());
				if ( m_rawAudioBytes > 0 )
					osd_printf_verbose("Audio codec ratio: %.1f%% [%s / %s]\n",
							   100.0 * ((double)m_encodedAudioBytes / (double)m_rawAudioBytes),
							   human_readable_value(m_encodedAudioBytes).toLocal8Bit().constData(),
							   human_readable_value(m_rawAudioBytes).toLocal8Bit().constData());
				m_frameCounter = m_encodedAudioBytes = m_rawAudioBytes = 0;
				m_frameChangePercent = 0.0;
			}
		}

		rfbResetPause = 0;
	}
}

//============================================================
//  find_modified_quads
//============================================================

QList<QRect> &vnc_osd_interface::find_modified_quads()
{
	m_modifiedQuads.clear();
	if ( rfbShadowValid ) {
		// find changed quads
		for (int y_quad = 0; y_quad < m_maxQuadsY; y_quad++) {
			int32_t yy_quad = y_quad * VNC_OSD_UPDATE_QUAD_SIZE;
			for (int x_quad = 0; x_quad < m_maxQuadsX; x_quad++) {
				int32_t xx_quad = x_quad * VNC_OSD_UPDATE_QUAD_SIZE;
				bool modified = false;
				for (int32_t y = yy_quad; y < yy_quad + VNC_OSD_UPDATE_QUAD_SIZE && y < rfbFrameBufferHeight; y++) {
					int32_t pos = y * rfbScanLineSize + xx_quad * VNC_OSD_BYTES_PER_PIXEL;
					for (int32_t x = xx_quad; x < xx_quad + VNC_OSD_UPDATE_QUAD_SIZE && x < rfbFrameBufferWidth; x++) {
						if ( *(int32_t *)(rfbShadowFrameBuffer + pos) != *(int32_t *)(rfbScreen->frameBuffer + pos) ) {
							modified = true;
							break;
						}
						pos += VNC_OSD_BYTES_PER_PIXEL;
					}
					if ( modified )
						break;
				}
				if ( modified ) {
					int32_t w, h;
					if ( yy_quad + VNC_OSD_UPDATE_QUAD_SIZE < rfbFrameBufferHeight )
						h = VNC_OSD_UPDATE_QUAD_SIZE;
					else
						h = rfbFrameBufferHeight - yy_quad;
					if ( xx_quad + VNC_OSD_UPDATE_QUAD_SIZE < rfbFrameBufferWidth )
						w = VNC_OSD_UPDATE_QUAD_SIZE;
					else
						w = rfbFrameBufferWidth - xx_quad;
					m_modifiedQuads << QRect(xx_quad, yy_quad, w, h);
				}
			}
		}
	} else
		// the shadow frame buffer is invalid, so force a full update
		m_modifiedQuads << QRect(0, 0, rfbFrameBufferWidth, rfbFrameBufferHeight);
	return m_modifiedQuads;
}

//============================================================
//  init_audio
//============================================================

void vnc_osd_interface::init_audio()
{
	m_encodedAudioBytes = m_rawAudioBytes = 0;
	avcodec_register_all();
	codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
	if ( !codec )
		osd_printf_verbose("Audio: MP2 codec not found\n");
	else {
		codecContext = avcodec_alloc_context3(codec);
		if ( !codecContext )
			osd_printf_verbose("Audio: Could not allocate MP2 codec context\n");
		else {
			codecContext->codec_type = AVMEDIA_TYPE_AUDIO;
			codecContext->bit_rate = m_options.vnc_audio_bitrate();
			codecContext->sample_rate = m_options.sample_rate();
			codecContext->channels = 2;
			codecContext->channel_layout = AV_CH_LAYOUT_STEREO;
			codecContext->sample_fmt = AV_SAMPLE_FMT_S16;
			codecContext->time_base = (AVRational){1, codecContext->sample_rate};
			if ( avcodec_open2(codecContext, codec, NULL) < 0 ) {
				osd_printf_verbose("Audio: Could not open MP2 codec\n");
				avcodec_close(codecContext);
				av_free(codecContext);
				codecContext = 0;
			} else {
				m_encoderBufferSize = av_samples_get_buffer_size(NULL, codecContext->channels, codecContext->frame_size, codecContext->sample_fmt, 0);
				osd_printf_verbose("Audio: MP2 codec successfully opened\n");
				if ( m_options.vnc_mp2write() && !mp2File ) {
					mp2File = fopen("mame_audio_stream.mp2", "w");
					if ( mp2File )
						osd_printf_verbose("Audio: MP2 output file successfully opened\n");
					else
						osd_printf_verbose("Audio: Could not open MP2 output file\n");
				}
				if ( !audio_server() ) {
					m_audioServer = new AudioServerThread(m_options.vnc_audio_port(), m_options.sample_rate(), m_options.vnc_audio_maxconn());
					audio_server()->start();
				}
			}
		}
	}
}

//============================================================
//  update_audio_stream
//============================================================

void vnc_osd_interface::update_audio_stream(const int16_t *buffer, int samples_this_frame)
{
	// buffer contains 16-bit L-R stereo samples (each stereo sample is layed out as 'LLRR' - 2 bytes for the left channel's sample, then 2 bytes for the right channel's sample)
	if ( m_options.sample_rate() != 0 && codecContext ) {
		uint32_t bytes_this_frame = samples_this_frame * sizeof(int16_t) * 2;
		m_queuedAudioData.append((const char *)buffer, bytes_this_frame);
		if ( m_queuedAudioData.size() < m_encoderBufferSize )
			return;
		while ( m_queuedAudioData.size() > m_encoderBufferSize )
		{
			AVFrame *frame = av_frame_alloc();
			frame->nb_samples = codecContext->frame_size;
			frame->format = codecContext->sample_fmt;
			frame->channel_layout = codecContext->channel_layout;
			int ret = avcodec_fill_audio_frame(frame, codecContext->channels, codecContext->sample_fmt, (const uint8_t *)m_queuedAudioData.constData(), m_encoderBufferSize, 0);
			if ( ret >= 0 ) {
				AVPacket pkt;
				av_init_packet(&pkt);
				pkt.data = NULL; // set by encoder
				pkt.size = 0;
				int got_output = 0;
				ret = avcodec_encode_audio2(codecContext, &pkt, frame, &got_output);
				if ( ret >= 0 && got_output ) {
					m_encodedAudioBytes += pkt.size;
					if ( mp2File )
						fwrite(pkt.data, 1, pkt.size, mp2File);
					if ( m_machine->video().throttled() && !m_machine->video().fastforward() )
						audio_server()->enqueueDatagram(QByteArray((const char *)pkt.data, pkt.size));
					av_free_packet(&pkt);
				}
			}
			m_queuedAudioData.remove(0, m_encoderBufferSize);
			av_frame_free(&frame);
			m_rawAudioBytes += m_encoderBufferSize;
		}
	}
}

//============================================================
//  set_mastervolume
//============================================================

void vnc_osd_interface::set_mastervolume(int attenuation)
{
	// if we had actual sound output, we would adjust the global
	// volume in response to this function
}

//============================================================
//  customize_input_type_list
//============================================================

void vnc_osd_interface::customize_input_type_list(simple_list<input_type_entry> &typelist)
{
	// This function is called on startup, before reading the
	// configuration from disk. Scan the list, and change the
	// default control mappings you want. It is quite possible
	// you won't need to change a thing.
	for (input_type_entry *entry = typelist.first(); entry != 0; entry = entry->next()) {
		switch ( entry->type() ) {
			// PageDown for "fast-forward"
			case IPT_UI_FAST_FORWARD:
				entry->defseq(SEQ_TYPE_STANDARD).set(KEYCODE_PGDN);
				break;
			default:
				break;
		}
	}
}

//============================================================
//  RFB related
//============================================================

void vnc_osd_interface::rfbProcessMouseEvent(int buttonMask, int x, int y, rfbClientPtr /*rfb_client*/)
{
	vnc_input_set_mouse_state(buttonMask, x, y);
}

void vnc_osd_interface::rfbProcessKeyEvent(rfbBool pressed, rfbKeySym key, rfbClientPtr /*rfb_client*/)
{
	vnc_input_set_key_state(pressed, key);
	if ( pressed )
		m_machine->ui_input().push_char_event(vnc_render_target, keysym2ucs(key));
}

void vnc_osd_interface::rfbProcessClientDisconnect(rfbClientPtr /*rfb_client*/)
{
	rfbClientCounter--;
	if ( rfbAutoPause && rfbClientCounter < 1 ) {
		if ( !m_machine->paused() ) {
			osd_printf_verbose("Auto-pausing machine\n");
			m_machine->pause();
			rfbWasAutoPaused = true;
		}
	}
}

enum rfbNewClientAction vnc_osd_interface::rfbProcessNewClient(rfbClientPtr rfb_client)
{
	if ( rfbAutoPause && rfbWasAutoPaused ) {
		osd_printf_verbose("Auto-resuming machine\n");
		m_machine->resume();
	}
	rfbWasAutoPaused = false;
	rfbClientCounter++;
	rfb_client->clientGoneHook = rfbProcessClientDisconnect;
	rfbRefreshFrameBuffer = osd_ticks();
	return RFB_CLIENT_ACCEPT;
}

void vnc_osd_interface::rfbNewFrameBuffer(int width, int height)
{
	char *oldFB, *oldShadowFB, *newFB, *newShadowFB;
	oldFB = rfbScreen->frameBuffer;
	oldShadowFB = rfbShadowFrameBuffer;
	uint32_t newBufferSize = width * height * VNC_OSD_BYTES_PER_PIXEL;
	newFB = (char *)malloc(newBufferSize);
	newShadowFB = (char *)malloc(newBufferSize);
	memset(newFB, 0, newBufferSize);
	memset(newShadowFB, 0, newBufferSize);
	rfbNewFramebuffer(rfbScreen, newFB, width, height, VNC_OSD_BITS_PER_SAMPLE, VNC_OSD_SAMPLES_PER_PIXEL, VNC_OSD_BYTES_PER_PIXEL);
	rfbLog("New frame buffer dimensions: %dx%d\n", width, height);
	rfbShadowFrameBuffer = newShadowFB;
	rfbShadowValid = false;
	free(oldFB);
	free(oldShadowFB);

	// pre-calculate the checkerboard dimensions
	m_maxQuadsX = width / VNC_OSD_UPDATE_QUAD_SIZE;
	if ( width % VNC_OSD_UPDATE_QUAD_SIZE > 0 )
		m_maxQuadsX++;
	m_maxQuadsY = height / VNC_OSD_UPDATE_QUAD_SIZE;
	if ( height % VNC_OSD_UPDATE_QUAD_SIZE > 0 )
		m_maxQuadsY++;
}

void vnc_osd_interface::rfbLog(const char *format, ...)
{
	va_list args;
	QString message;
	va_start(args, format);
	message.vsprintf(format, args);
	va_end(args);
	osd_printf_verbose("RFB: %s", message.toLocal8Bit().constData());
}

void vnc_osd_interface::output_callback(osd_output_channel channel, const char *msg, va_list args)
{
	osd_common_t::output_callback(channel, msg, args);
}

QString &vnc_osd_interface::human_readable_value(double value)
{
	static QString humanReadableString;
	static QLocale locale;
	if ( value / VNC_OSD_ONE_KILOBYTE < VNC_OSD_ONE_KILOBYTE ) {
		humanReadableString = locale.toString(value / (double)VNC_OSD_ONE_KILOBYTE, 'f', 2) + " KB";
		return humanReadableString;
	}
	if ( value / VNC_OSD_ONE_MEGABYTE < VNC_OSD_ONE_KILOBYTE ) {
		humanReadableString = locale.toString(value / (double)VNC_OSD_ONE_MEGABYTE, 'f', 2) + " MB";
		return humanReadableString;
	}
	humanReadableString = locale.toString(value / VNC_OSD_ONE_GIGABYTE, 'f', 2) + " GB";
	return humanReadableString;
}
