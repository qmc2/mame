#ifndef AUDIOSERVER_H
#define AUDIOSERVER_H

#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <QHash>
#include <QQueue>

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
	Q_OBJECT

public:
	explicit AudioServerThread(QObject *parent = 0);
	~AudioServerThread();

	QUdpSocket *socket() { return m_socket; }
	void setSocket(QUdpSocket *socket) { m_socket = socket; }
	QHostAddress &localAddress() { return m_localAddress; }
	void setLocalAddress(const QHostAddress &localAddress) { m_localAddress = localAddress; }
	int localPort() { return m_localPort; }
	void setLocalPort(int localPort) { m_localPort = localPort; }
	void exitThread() { m_exit = true; }
	void sendDatagram(const QByteArray &datagram);
	QHash<QByteArray, UdpConnection> &connections() { return m_connections; }
	bool bindToLocalPort();

public slots:
	void error(QAbstractSocket::SocketError);
	void sendQueuedDatagrams();
	void enqueueDatagram(const QByteArray &datagram);

protected:
	void run();

private:
	QUdpSocket *m_socket;
	QHostAddress m_localAddress;
	int m_localPort;
	bool m_exit;
	QHash<QByteArray, UdpConnection> m_connections;
	QQueue<QByteArray> m_sendQueue;
};

#endif // AUDIOSERVER_H
