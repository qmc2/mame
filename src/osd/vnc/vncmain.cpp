// license:BSD-3-Clause
// copyright-holders:MAMEdev Team

#include <QByteArray>
#include <QString>
#include <QLocale>
#include <QUrl>
#include <QMetaType>
#include <QApplication>
#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
//#include <QQuickView>
//#include <QQuickItem>
//#include <QQmlContext>
#include <QHostInfo>

#include "emu.h"
#include "osdepend.h"
#include "mame.h"
#include "clifront.h"
#include "inputdev.h"
#include "render.h"
#include "uiinput.h"
#include "keysym2ucs.h"
#include "keysym2qt.h"
#include "osdvnc.h"
#include <string.h>
#include <unistd.h>
#include <vncinput.h>
#include "rendersw.hxx"

//#include "roleitemmodel.h"

// min/max of two constants
#define MAX(a, b)	(((a) > (b)) ? (a) : (b))
#define MIN(a, b)	(((a) < (b)) ? (a) : (b))

running_machine *vnc_osd_interface::m_machine = 0;

//============================================================
//  TYPES
//============================================================

struct ConnectionEntry {
	enum ConnectionRoles { IdRole = Qt::UserRole + 1, AddressRole, HostnameRole, PortRole };
};

const char *emulator_name() { return emulator_info::get_appname(); }

//============================================================
//  CONSTANTS
//============================================================

#define VNC_OSD_BITS_PER_SAMPLE		8
#define VNC_OSD_SAMPLES_PER_PIXEL	1
#define VNC_OSD_BYTES_PER_PIXEL		4
#define VNC_OSD_MOUSE_MOVE_TIMEOUT	1000	// OSD ticks
#define VNC_OSD_REFRESH_TIMEOUT		200000	// OSD ticks
#define VNC_OSD_DOUBLECLICK_TIMEOUT	250000	// OSD ticks
#define VNC_OSD_PERFINFO_FRAMES		60	// output performance-data every 60 frames
#define VNC_OSD_ONE_KILOBYTE		1024.0f
#define VNC_OSD_ONE_MEGABYTE		1048576.0f
#define VNC_OSD_ONE_GIGABYTE		1073741824.0f

//============================================================
//  MACROS
//============================================================

// make a string out of a non-string constant
#define STR(s)				#s
#define XSTR(s)				STR(s)

//============================================================
//  GLOBALS
//============================================================

// a single rendering target
static render_target *vnc_render_target = 0;

// input devices
static input_device *keyboard_device = 0;
static input_device *mouse_device = 0;

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
vnc_osd_interface *vnc_osd = 0;

// overlay GUI related
/*
QApplication *vncOsdApp = 0;
QQuickView *vncOsdOverlayUi = 0;
QSize vncOsdUiSceneSize;
QRect vncOsdUiScaledRect;
RoleItemModel *vncOsdConnectionModel = 0;
bool vncOsdOverlayUiActive = false;
osd_ticks_t vncOsdLastMouseClick = 0;
bool vncOsdLastUiActive = false;
bool vncOsdWasUiPaused = false;
*/

// log mutex
MUTEX(vncOsdLogMutex);

//extern rfbBool rfbLeaveCriticalSection;

const options_entry vnc_options::vnc_option_entries[] =
{
	{ 0,			0,		OPTION_HEADER,		"VNC OPTIONS" },
	{ "vnc_port",		"5900",		OPTION_INTEGER,		"TCP port to listen on for incoming VNC connections (default: 5900)" },
	{ "vnc_adjust_fb",	"1",		OPTION_BOOLEAN,		"Auto-adjust the frame-buffer width to be a multiple of 4 for best client compatibility (default: 1)" },
	{ "vnc_autopause",	"1",		OPTION_BOOLEAN,		"Pause the machine when all clients disconnected, resume it when a client connects (default: 1)" },

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

    	INIT_MUTEX(vncOsdLogMutex);

	// instantiate options and OSD
	vnc_options options;
	vnc_osd = new vnc_osd_interface(options);
	vnc_osd->register_options();

	// initialize the VNC OSD overlay UI
	/*
	vncOsdApp = new QApplication(argc, argv);
	qRegisterMetaType<QModelIndex>("QModelIndex"); // this is weird but necessary to satisfy the QML layer
	vncOsdOverlayUi = new QQuickView;
	vncOsdOverlayUi->rootContext()->setContextProperty("connectionModel", 0);
	vncOsdOverlayUi->setSource(QUrl::fromLocalFile("src/osd/vnc/qml/ui_overlay.qml"));
	vncOsdOverlayUi->rootObject()->setProperty("windowTitle", QString("VNC OSD interface v%1").arg(XSTR(VNC_OSD_VERSION)));
	vncOsdOverlayUi->scene()->setBackgroundBrush(Qt::transparent);
	vncOsdOverlayUi->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
	vncOsdOverlayUi->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	vncOsdOverlayUi->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	vncOsdUiSceneSize = vncOsdOverlayUi->scene()->sceneRect().size().toSize();
	*/

	// run the actual emulation
	cli_frontend *frontend = new cli_frontend(options, *vnc_osd);
	int returnCode = frontend->execute(argc, argv);
	
	// clean up
	if ( rfbScreen ) {
		rfbScreenCleanup(rfbScreen);
		free(rfbShadowFrameBuffer);
	}
	delete frontend;
	delete vnc_osd;
	return returnCode;
}

//============================================================
//  human_readable_value
//============================================================

QString human_readable_value(double value)
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

//============================================================
//  vnc_osd_output_callback
//============================================================

void vnc_osd_output_callback(const char *msg, va_list args)
{
	vnc_osd->output_callback(OSD_OUTPUT_CHANNEL_LOG, msg, args);
}

//============================================================
//  constructor
//============================================================

vnc_osd_interface::vnc_osd_interface(vnc_options &options)
	: osd_common_t(options), m_options(options), m_frameCounter(0), m_framePercent(0.0), m_frameBufferSize(0.0)
{
}

//============================================================
//  destructor
//============================================================

vnc_osd_interface::~vnc_osd_interface()
{
	/*
	if ( vncOsdOverlayUi )
		delete vncOsdOverlayUi;
	if ( vncOsdApp )
		delete vncOsdApp;
	*/
}

//============================================================
//  init
//============================================================

void vnc_osd_interface::init(running_machine &machine)
{
	// call our parent
	osd_common_t::init(machine);
	m_machine = &machine;

	// log banner
	osd_printf_verbose("VNC-OSD-INFO: VNC OSD interface v%s\n", XSTR(VNC_OSD_VERSION));

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
		rfbScale = MAX(1, options.prescale());
	}

	// initialize the video system by allocating a rendering target and setting a view
	vnc_render_target = machine.render().target_alloc();

	for (const screen_device &screen : screen_device_iterator(machine.root_device()))
		osd_printf_verbose("VNC-OSD-INFO: Screen #%d: %dx%d\n", rfbScreenCount++, screen.width(), screen.height());

	// we need to call this initially -- before setting the actual view (!) -- to get a sane state on the *configured*
	// view, otherwise machine-config saving (with regard to the layout-view) won't work correctly on machine exit
	vnc_render_target->get_primitives();

	// now configure the view (from stored config)
	QString view_name(options.view());
	if ( view_name != "auto" )
		vnc_render_target->set_view(vnc_render_target->configured_view(options.view(), 0, 1));
	else
		vnc_render_target->set_view(1); // auto => "pixel aspect"

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
	// vncOsdWasUiPaused = vncOsdLastUiActive = vncOsdOverlayUiActive = false;
}

//============================================================
//  osd_update
//============================================================

void vnc_osd_interface::update(bool skip_redraw)
{
	// get the minimum width/height for the current layout and make that the (scaled) size of our target
	vnc_render_target->compute_minimum_size(rfbFrameBufferWidth, rfbFrameBufferHeight);
	rfbFrameBufferWidth *= rfbScale; rfbFrameBufferHeight *= rfbScale;
	if ( rfbFrameBufferWidth > rfbFrameBufferHeight ) {
		int32_t temp = rfbFrameBufferWidth;
		rfbFrameBufferWidth = rfbFrameBufferHeight;
		rfbFrameBufferHeight = temp;
	}
	vnc_render_target->compute_visible_area(rfbFrameBufferWidth, rfbFrameBufferHeight, rfbFrameBufferHeight / ((rfbScreenCount > 0 ? rfbScreenCount : 1) * rfbFrameBufferWidth), vnc_render_target->orientation(), rfbFrameBufferWidth, rfbFrameBufferHeight);
	vnc_render_target->set_bounds(rfbFrameBufferWidth, rfbFrameBufferHeight);

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

		/*
		checkUiState();
		if ( vncOsdOverlayUiActive ) {
			// update & render the VNC OSD overlay UI
			vncOsdApp->processEvents();
			vncOsdOverlayUi->update();
			m_vncOsdUiImage = QImage(vncOsdUiSceneSize, QImage::Format_ARGB32);
			m_vncOsdUiImage.fill(Qt::transparent);
			QPainter p(&m_vncOsdUiImage);
			vncOsdOverlayUi->scene()->render(&p);
			p.end();
			m_vncOsdUiImage = m_vncOsdUiImage.scaled(rfbFrameBufferWidth - 10, rfbFrameBufferHeight - 10, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			uint32_t xOffset = (rfbFrameBufferWidth - m_vncOsdUiImage.width()) / 2;
			uint32_t yOffset = (rfbFrameBufferHeight - m_vncOsdUiImage.height()) / 2;
			vncOsdUiScaledRect = QRect(xOffset, yOffset, m_vncOsdUiImage.width(), m_vncOsdUiImage.height());
			for (uint32_t y = 0; y < m_vncOsdUiImage.height(); y++) {
				uint32_t yPos = (y + yOffset) * rfbScanLineSize;
				for (uint32_t x = 0; x < m_vncOsdUiImage.width(); x++) {
					QRgb pix = m_vncOsdUiImage.pixel(x, y);
					if ( qAlpha(pix) != 0 ) {
						uint32_t pos = yPos + (x + xOffset) * VNC_OSD_BYTES_PER_PIXEL;
#if defined(VNC_OSD_BIGENDIAN)
						rfbScreen->frameBuffer[pos + 1] = qRed(pix);
						rfbScreen->frameBuffer[pos + 2] = qGreen(pix);
						rfbScreen->frameBuffer[pos + 3] = qBlue(pix);
#else
						rfbScreen->frameBuffer[pos + 0] = qRed(pix);
						rfbScreen->frameBuffer[pos + 1] = qGreen(pix);
						rfbScreen->frameBuffer[pos + 2] = qBlue(pix);
#endif
					}
				}
			}
		}
		*/

		int32_t x1 = rfbFrameBufferWidth,
			y1 = rfbFrameBufferHeight,
			x2 = -1,
			y2 = -1;

		if ( rfbShadowValid ) {
			// find the outer rectangle of changed pixels
			uint32_t pos = 0;
			for (int32_t y = 0; y < rfbFrameBufferHeight; y++) {
				for (int32_t x = 0; x < rfbFrameBufferWidth; x++) {
					if ( *(int32_t *)(rfbShadowFrameBuffer + pos) != *(int32_t *)(rfbScreen->frameBuffer + pos) ) {
						x1 = MIN(x, x1);
						y1 = MIN(y, y1);
						x2 = MAX(x, x2);
						y2 = MAX(y, y2);
					}
					pos += VNC_OSD_BYTES_PER_PIXEL;
				}
			}
		} else {
			// the shadow frame buffer is invalid, so force a full update
			x1 = y1 = 0;
			x2 = rfbFrameBufferWidth - 1;
			y2 = rfbFrameBufferHeight - 1;
		}

		bool modified = (x2 >= x1 && y2 >= y1);

		if ( modified ) {
			rfbMarkRectAsModified(rfbScreen, x1, y1, x2 + 1, y2 + 1);
			memcpy(rfbShadowFrameBuffer, rfbScreen->frameBuffer, rfbBufferSize);
			rfbShadowValid = true;
		}

		if ( rfbVerbose ) {
			if ( modified )
				m_framePercent += (double)((x2 - x1) * (y2 - y1)) / m_frameBufferSize;
			m_frameCounter++;
			if ( m_frameCounter >= VNC_OSD_PERFINFO_FRAMES ) {
				double frameTotalBytes = (double)m_frameCounter * (double)rfbBufferSize;
				m_framePercent /= (double)m_frameCounter;
				osd_printf_verbose("VNC-OSD-INFO: Raw frame-buffer updates (last %d frames): %.2f%% [%s / %s]\n",
						   m_frameCounter,
						   100.0 * m_framePercent,
						   human_readable_value(frameTotalBytes * m_framePercent).toLocal8Bit().constData(),
						   human_readable_value(frameTotalBytes).toLocal8Bit().constData());
				m_frameCounter = 0;
				m_framePercent = 0.0;
			}
		}

		rfbResetPause = 0;
	}
}

//============================================================
//  update_audio_stream
//============================================================

void vnc_osd_interface::update_audio_stream(const int16_t *buffer, int samples_this_frame)
{
	// if we had actual sound output, we would copy the
	// interleaved stereo samples to our sound stream
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
	//static int currentUiButtonMask = 0;

	vnc_input_set_mouse_state(buttonMask, x, y);
	/*
	if ( !vncOsdOverlayUiActive )
		vnc_input_set_mouse_state(buttonMask, x, y);
	else if ( vncOsdUiScaledRect.contains(x, y, true) && buttonMask != currentUiButtonMask ) {
		// post a synthetic mouse-event to the UI (we don't post *pure* move-events, though)
		Qt::MouseButtons qtButtonState = Qt::NoButton;
		if ( buttonMask & 0x1 )
			qtButtonState |= Qt::LeftButton;
		if ( buttonMask & 0x2 )
			qtButtonState |= Qt::MiddleButton;
		if ( buttonMask & 0x4 )
			qtButtonState |= Qt::RightButton;
		int diffButtonMask = buttonMask ^ currentUiButtonMask;
		Qt::MouseButton qtRelevantButton = Qt::NoButton;
		QEvent::Type qtEventType = QEvent::None;
		bool isDoubleClick = (osd_ticks() - vncOsdLastMouseClick < VNC_OSD_DOUBLECLICK_TIMEOUT) && (buttonMask & 0x1);
		if ( diffButtonMask & 0x1 ) {
			qtRelevantButton = Qt::LeftButton;
			qtEventType = buttonMask & 0x1 ? (isDoubleClick ? QEvent::GraphicsSceneMouseDoubleClick : QEvent::GraphicsSceneMousePress) : QEvent::GraphicsSceneMouseRelease;
		} else if ( diffButtonMask & 0x2 ) {
			qtRelevantButton = Qt::MiddleButton;
			qtEventType = buttonMask & 0x2 ? QEvent::GraphicsSceneMousePress : QEvent::GraphicsSceneMouseRelease;
		} else if ( diffButtonMask & 0x4 ) {
			qtRelevantButton = Qt::RightButton;
			qtEventType = buttonMask & 0x4 ? QEvent::GraphicsSceneMousePress : QEvent::GraphicsSceneMouseRelease;
		}
		if ( qtRelevantButton != Qt::NoButton ) {
			QGraphicsSceneMouseEvent *emulatedMouseEvent = new QGraphicsSceneMouseEvent(qtEventType);
			double xScale = (double)vncOsdUiSceneSize.width() / (double)vncOsdUiScaledRect.width();
			double yScale = (double)vncOsdUiSceneSize.height() / (double)vncOsdUiScaledRect.height();
			emulatedMouseEvent->setScenePos(QPointF((x - vncOsdUiScaledRect.x()) * xScale, (y - vncOsdUiScaledRect.y()) * yScale));
			emulatedMouseEvent->setButton(qtRelevantButton);
			emulatedMouseEvent->setButtons(qtButtonState);
			vncOsdApp->postEvent(vncOsdOverlayUi->scene(), emulatedMouseEvent);
		}
		currentUiButtonMask = buttonMask;
		vncOsdLastMouseClick = buttonMask & 0x1 ? osd_ticks() : vncOsdLastMouseClick;
	}
	*/
}

void vnc_osd_interface::rfbProcessKeyEvent(rfbBool pressed, rfbKeySym key, rfbClientPtr /*rfb_client*/)
{
	vnc_input_set_key_state(pressed, key);
	if ( pressed )
		m_machine->ui_input().push_char_event(vnc_render_target, (char32_t) keysym2ucs(key));
	/*
	if ( !vncOsdOverlayUiActive ) {
		if ( pressed )
			ui_input_push_char_event(*m_machine, vnc_render_target, keysym2ucs(key));
	} else {
		// post a synthetic key-event to the UI
		QKeyEvent *emulatedKeyEvent = new QKeyEvent(pressed ? QEvent::KeyPress : QEvent::KeyRelease, keysym2qt(key), Qt::NoModifier);
		vncOsdApp->postEvent(vncOsdOverlayUi, emulatedKeyEvent);
	}
	*/
}

void vnc_osd_interface::rfbProcessClientDisconnect(rfbClientPtr rfb_client)
{
	rfbClientCounter--;
	if ( rfbAutoPause && rfbClientCounter < 1 ) {
		if ( !m_machine->paused() ) {
			osd_printf_verbose("VNC-OSD-INFO: Auto-pausing machine\n");
			m_machine->pause();
			rfbWasAutoPaused = true;
		}
	}

	/*
	QString id = QString::number((quint64)rfb_client, 16);
	for (int row = 0; row < vncOsdConnectionModel->rowCount(); row++) {
		QStandardItem *item = vncOsdConnectionModel->item(row);
		if ( item->data(ConnectionEntry::IdRole) == id ) {
			vncOsdConnectionModel->removeRow(row);
			break;
		}
	}
	*/
}

enum rfbNewClientAction vnc_osd_interface::rfbProcessNewClient(rfbClientPtr rfb_client)
{
	/*
	if ( !vncOsdConnectionModel ) {
		QHash<int, QByteArray> roleNames;
		roleNames[ConnectionEntry::IdRole] = "id";
		roleNames[ConnectionEntry::AddressRole] = "address";
		roleNames[ConnectionEntry::HostnameRole] = "hostname";
		roleNames[ConnectionEntry::PortRole] = "port";
		vncOsdConnectionModel = new RoleItemModel(roleNames);
		vncOsdOverlayUi->rootContext()->setContextProperty("connectionModel", vncOsdConnectionModel);
	}
	QStandardItem* connectionItem = new QStandardItem();
	connectionItem->setData(QString::number((quint64)rfb_client, 16), ConnectionEntry::IdRole);
	connectionItem->setData(QString(rfb_client->host), ConnectionEntry::AddressRole);
	connectionItem->setData(QHostInfo::fromName(QString(rfb_client->host)).hostName(), ConnectionEntry::HostnameRole);
	struct sockaddr_in sa;
	socklen_t sa_len = sizeof(sa);
#if defined(VNC_OSD_WINDOWS)
	if ( getpeername(rfb_client->sock, (LPSOCKADDR)&sa, &sa_len) == 0 )
		connectionItem->setData(QString::number(ntohs(sa.sin_port)), ConnectionEntry::PortRole);
	else
		connectionItem->setData("error", ConnectionEntry::PortRole);
#else
	if ( getpeername(rfb_client->sock, (sockaddr*)&sa, &sa_len) == 0 )
		connectionItem->setData(QString::number(ntohs(sa.sin_port)), ConnectionEntry::PortRole);
	else
		connectionItem->setData("error", ConnectionEntry::PortRole);
#endif
	vncOsdConnectionModel->appendRow(connectionItem);
	*/

	if ( rfbAutoPause && rfbWasAutoPaused ) {
		osd_printf_verbose("VNC-OSD-INFO: Auto-resuming machine\n");
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
	//rfbLeaveCriticalSection = true;
	char *oldFB, *oldShadowFB, *newFB, *newShadowFB;
	oldFB = rfbScreen->frameBuffer;
	oldShadowFB = rfbShadowFrameBuffer;
	uint32_t newBufferSize = width * height * VNC_OSD_BYTES_PER_PIXEL;
	newFB = (char *)malloc(newBufferSize);
	newShadowFB = (char *)malloc(newBufferSize);
	memset(newFB, 0, newBufferSize);
	memset(newShadowFB, 0, newBufferSize);
	rfbNewFramebuffer(rfbScreen, newFB, width, height, VNC_OSD_BITS_PER_SAMPLE, VNC_OSD_SAMPLES_PER_PIXEL, VNC_OSD_BYTES_PER_PIXEL);
	rfbShadowFrameBuffer = newShadowFB;
	rfbShadowValid = false;
	free(oldFB);
	free(oldShadowFB);
	//rfbLeaveCriticalSection = false;
}

/*
void vnc_osd_interface::checkUiState()
{
	if ( vncOsdOverlayUiActive && vncOsdOverlayUi->rootObject()->property("exitRequested").toBool() ) {
		vncOsdOverlayUiActive = false;
		vncOsdOverlayUi->rootObject()->setProperty("exitRequested", false);
	}

	if ( vncOsdLastUiActive != vncOsdOverlayUiActive ) {
		if ( vncOsdLastUiActive && m_machine->paused() && vncOsdWasUiPaused ) {
			osd_printf_verbose("VNC-OSD-INFO: Auto-resuming machine\n");
			m_machine->resume();
			vncOsdWasUiPaused = false;
		} else if ( !m_machine->paused() ) {
			osd_printf_verbose("VNC-OSD-INFO: Auto-pausing machine\n");
			m_machine->pause();
			vncOsdWasUiPaused = true;
		} else
			vncOsdWasUiPaused = false;
	}

	vncOsdLastUiActive = vncOsdOverlayUiActive;
}
*/

void vnc_osd_interface::output_callback(osd_output_channel channel, const char *msg, va_list args)
{
	// we need 'serialized' output because log messages can come from different threads
	LOCK(vncOsdLogMutex);
	osd_common_t::output_callback(channel, msg, args);
	UNLOCK(vncOsdLogMutex);
}
