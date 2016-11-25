#include <QDateTime>
#include "audioserver.h"
#include "emu.h"

AudioServerThread::AudioServerThread(QObject *parent) :
	QThread(parent),
	m_socket(0),
	m_localAddress(QHostAddress::Any),
	m_localPort(6900), // FIXME
	m_exit(false)
{
	qRegisterMetaType<QAbstractSocket::SocketError>();
	start();
}

AudioServerThread::~AudioServerThread()
{
	exitThread();
	wait();
}

bool AudioServerThread::bindToLocalPort()
{
	if ( socket()->bind(localAddress(), localPort()) ) {
		osd_printf_verbose("Audio: server socket bound to address %s / port %d\n", localAddress().toString().toLocal8Bit().constData(), localPort());
		return true;
	} else {
		osd_printf_verbose("Audio: couldn't bind server socket to address %s / port %d: %s\n", localAddress().toString().toLocal8Bit().constData(), localPort(), socket()->errorString().toLower().toLocal8Bit().constData());
		return false;
	}
}

void AudioServerThread::error(QAbstractSocket::SocketError)
{
	osd_printf_verbose("Audio: socket error: %s\n", socket()->errorString().toLower().toLocal8Bit().constData());
}

void AudioServerThread::sendDatagram(const QByteArray &datagram)
{
	QHashIterator<QByteArray, UdpConnection> iter(connections());
	while ( iter.hasNext() ) {
		iter.next();
		socket()->writeDatagram(datagram, iter.value().address, iter.value().port);
	}
}

void AudioServerThread::enqueueDatagram(const QByteArray &datagram)
{
	m_sendQueue.enqueue(datagram);
}

void AudioServerThread::sendQueuedDatagrams()
{
	while ( !m_exit && !m_sendQueue.isEmpty() )
		sendDatagram(m_sendQueue.dequeue());
}

void AudioServerThread::run()
{
	setSocket(new QUdpSocket(0));
	connect(socket(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
	if ( bindToLocalPort() ) {
		while ( !m_exit ) {
			sendQueuedDatagrams();
			QThread::msleep(1);
		}
	}
	delete socket();
}
