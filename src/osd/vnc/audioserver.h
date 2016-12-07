#ifndef AUDIOSERVER_H
#define AUDIOSERVER_H

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <QHash>
#include <QQueue>
#include <QMutex>

#define VNC_OSD_AUDIO_DEFAULT_PORT				6900
#define VNC_OSD_AUDIO_DEFAULT_MAX_CONNECTIONS			32
#define VNC_OSD_AUDIO_DEFAULT_SAMPLE_RATE			48000

#define VNC_OSD_AUDIO_COMMAND_IDX_CONNECT_TO_STREAM		0
#define VNC_OSD_AUDIO_COMMAND_IDX_DISCONNECT_FROM_STREAM	1
#define VNC_OSD_AUDIO_COMMAND_IDX_SAMPLE_RATE			2

#define VNC_OSD_AUDIO_COMMAND_STR_CONNECT_TO_STREAM		QByteArray("VNC_OSD_AUDIO_CONNECT_TO_STREAM")
#define VNC_OSD_AUDIO_COMMAND_STR_DISCONNECT_FROM_STREAM	QByteArray("VNC_OSD_AUDIO_DISCONNECT_FROM_STREAM")
#define VNC_OSD_AUDIO_COMMAND_STR_SAMPLE_RATE			QByteArray("VNC_OSD_AUDIO_SAMPLE_RATE")

class UdpConnection
{
public:
	QHostAddress address;
	quint16 port;

	UdpConnection() {}
	UdpConnection(const QHostAddress &a, const quint16 p) { address = a; port = p; }
	UdpConnection(const UdpConnection &o) { address = o.address; port = o.port; }
};

class AudioServerThread : public QThread
{
public:
	explicit AudioServerThread(int localPort = VNC_OSD_AUDIO_DEFAULT_PORT, int sampleRate = VNC_OSD_AUDIO_DEFAULT_SAMPLE_RATE, int maxConnections = VNC_OSD_AUDIO_DEFAULT_MAX_CONNECTIONS, QObject *parent = 0);
	~AudioServerThread();

	QUdpSocket *socket() { return m_socket; }
	void setSocket(QUdpSocket *socket) { m_socket = socket; }
	QHostAddress &localAddress() { return m_localAddress; }
	void setLocalAddress(const QHostAddress &localAddress) { m_localAddress = localAddress; }
	int localPort() { return m_localPort; }
	void setLocalPort(int localPort) { m_localPort = localPort; }
	int maxConnections() { return m_maxConnections; }
	void setMaxConnections(int maxConnections) { m_maxConnections = maxConnections; }
	void sendDatagram(const QByteArray &datagram);
	QHash<QString, UdpConnection> &connections() { return m_connections; }
	bool bindToLocalPort();
	void processDatagram(const QByteArray &datagram, const QHostAddress &peer, quint16 peerPort);
	void sendQueuedDatagrams();
	void enqueueDatagram(const QByteArray &datagram);
	void readPendingDatagrams();

protected:
	void run();

private:
	QUdpSocket *m_socket;
	QHostAddress m_localAddress;
	int m_localPort;
	int m_sampleRate;
	int m_maxConnections;
	bool m_exit;
	QHash<QString, UdpConnection> m_connections;
	QQueue<QByteArray> m_sendQueue;
	QStringList m_clientCommands;
	QMutex m_sendQueueMutex;
};

#endif // AUDIOSERVER_H
