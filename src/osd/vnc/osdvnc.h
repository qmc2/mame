// license:BSD-3-Clause
// copyright-holders:MAMEdev Team

#ifndef OSDVNC_H
#define OSDVNC_H

extern "C" {
#include <rfb/rfb.h>
}

#include "options.h"
#include "osdepend.h"
#include "osdcore.h"
#include "modules/lib/osdobj_common.h"
#include "modules/osdmodule.h"
#include "modules/font/font_module.h"
#include "audioserver.h"

#include <QByteArray>
#include <QString>
#include <QList>
#include <QRect>

//============================================================
//  TYPE DEFINITIONS
//============================================================

class vnc_options : public osd_options
{
public:
	// construction/destruction
	vnc_options();

	int vnc_port() const { return int_value("vnc_port"); }
	bool vnc_adjust_fb() const { return bool_value("vnc_adjust_fb"); }
	bool vnc_autopause() const { return bool_value("vnc_autopause"); }
	bool vnc_mp2write() const { return bool_value("vnc_mp2write"); }
	int vnc_audio_bitrate() const { return int_value("vnc_audio_bitrate"); } 
	int vnc_audio_port() const { return int_value("vnc_audio_port"); } 
	int vnc_audio_maxconn() const { return int_value("vnc_audio_maxconn"); } 

private:
	static const options_entry vnc_option_entries[];
};

class vnc_osd_interface : public osd_common_t
{
public:
	// construction/destruction
	vnc_osd_interface(vnc_options &options);
	virtual ~vnc_osd_interface();

	// general overridables
	virtual void init(running_machine &machine);
	virtual void update(bool skip_redraw);

	// audio overridables
	virtual void update_audio_stream(const int16_t *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);

	// input overridables
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist);

	// RFB related
	static void rfbProcessMouseEvent(int buttonMask, int x, int y, rfbClientPtr rfb_client);
	static void rfbProcessKeyEvent(rfbBool down, rfbKeySym key, rfbClientPtr rfb_client);
	static void rfbProcessClientDisconnect(rfbClientPtr rfb_client);
	static enum rfbNewClientAction rfbProcessNewClient(rfbClientPtr rfb_client);
	static void rfbNewFrameBuffer(int width, int height);
	static void rfbLog(const char *format, ...);

	void output_callback(osd_output_channel channel, const char *msg, va_list args);
	QString &human_readable_value(double value);
	QList<QRect> &find_modified_quads();
	void init_audio();
	AudioServerThread *audio_server() { return m_audioServer; }

signals:
	void enqueueAudioPacket(const QByteArray &packet);

private:
	static void vnc_exit(running_machine *);
	static running_machine *m_machine;
	vnc_options &m_options;
	int32_t m_frameCounter;
	double m_frameChangePercent;
	double m_frameBufferSize;
	int32_t m_rawAudioBytes;
	int32_t m_encodedAudioBytes;
	QList<QRect> m_modifiedQuads;
	static int32_t m_maxQuadsX, m_maxQuadsY;
	QByteArray m_queuedAudioData;
	uint32_t m_encoderBufferSize;
	AudioServerThread *m_audioServer;
	QString m_emptyDriverName;
};

//============================================================
//  GLOBAL VARIABLES
//============================================================

extern const options_entry mame_win_options[];

// defined in winwork.c
extern int osd_num_processors;

//============================================================
//  FUNCTION PROTOTYPES
//============================================================

// use if you want to print something with the verbose flag
void CLIB_DECL osd_printf_verbose(const char *text, ...) ATTR_PRINTF(1,2);

// use this to ping the watchdog
void winmain_watchdog_ping(void);
void winmain_dump_stack();

#endif
