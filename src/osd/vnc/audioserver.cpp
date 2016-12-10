#include "emu.h"
#include "audioserver.h"

#define clientId(peer, port)		QString("%1:%2").arg(peer.toString()).arg(port)

AudioServerThread::AudioServerThread(int localPort, int sampleRate, int maxConnections, QObject *parent) :
	QThread(parent),
	m_socket(0),
	m_localAddress(QHostAddress::Any),
	m_localPort(localPort),
	m_sampleRate(sampleRate),
	m_maxConnections(maxConnections),
	m_exit(false)
{
	m_clientCommands << VNC_OSD_AUDIO_COMMAND_STR_CONNECT_TO_STREAM << VNC_OSD_AUDIO_COMMAND_STR_DISCONNECT_FROM_STREAM;
	start();
}

AudioServerThread::~AudioServerThread()
{
	m_exit = true;
	wait();
}

bool AudioServerThread::bindToLocalPort()
{
	if ( socket()->bind(localAddress(), localPort()) ) {
		osd_printf_verbose("Audio Server: Socket bound to address %s / port %d\n", localAddress().toString().toLocal8Bit().constData(), localPort());
		return true;
	} else {
		osd_printf_verbose("Audio Server: Couldn't bind server socket to address %s / port %d: %s\n", localAddress().toString().toLocal8Bit().constData(), localPort(), socket()->errorString().toLower().toLocal8Bit().constData());
		return false;
	}
}

void AudioServerThread::readPendingDatagrams()
{
	while ( socket()->hasPendingDatagrams() ) {
		QByteArray datagram;
		datagram.resize(socket()->pendingDatagramSize());
		QHostAddress peer;
		quint16 peerPort; 
		socket()->readDatagram(datagram.data(), datagram.size(), &peer, &peerPort);
	        processDatagram(datagram, peer, peerPort);
	}
}

void AudioServerThread::processDatagram(const QByteArray &datagram, const QHostAddress &peer, quint16 peerPort)
{
	QString id(clientId(peer, peerPort));
	switch ( m_clientCommands.indexOf(datagram) ) {
		case VNC_OSD_AUDIO_COMMAND_IDX_CONNECT_TO_STREAM:
			if ( !connections().contains(id) ) {
				if ( connections().count() < maxConnections() ) {
					osd_printf_verbose("Audio Server: Connect from client at address %s / port %d - accepted\n", peer.toString().toLocal8Bit().constData(), peerPort);
					connections().insert(id, UdpConnection(peer, peerPort));
					socket()->writeDatagram(VNC_OSD_AUDIO_COMMAND_STR_SAMPLE_RATE + ' ' + QByteArray::number(m_sampleRate), peer, peerPort);
				} else {
					osd_printf_verbose("Audio Server: Connect from client at address %s / port %d - rejected (maximum number of connections reached)\n", peer.toString().toLocal8Bit().constData(), peerPort);
					socket()->writeDatagram(VNC_OSD_AUDIO_COMMAND_STR_CLIENT_REJECTED, peer, peerPort);
				}
			}
			break;
		case VNC_OSD_AUDIO_COMMAND_IDX_DISCONNECT_FROM_STREAM:
			if ( connections().contains(id) ) {
				osd_printf_verbose("Audio Server: Disconnect from client at address %s / port %d\n", peer.toString().toLocal8Bit().constData(), peerPort);
				connections().remove(id);
			}
			break;
		default:
			osd_printf_verbose("Audio Server: Unknown command received from %s client at address %s / port %d: '%s'\n", connections().contains(id) ? "connected" : "not connected", peer.toString().toLocal8Bit().constData(), peerPort, datagram.constData());
			break;
	}
}

void AudioServerThread::sendDatagram(const QByteArray &datagram)
{
	QHashIterator<QString, UdpConnection> iter(connections());
	while ( iter.hasNext() ) {
		iter.next();
		if ( socket()->writeDatagram(datagram, iter.value().address, iter.value().port) < 0 ) {
			osd_printf_verbose("Audio Server: Failed sending datagram to address %s / port %d\n", iter.value().address.toString().toLocal8Bit().constData(), iter.value().port);
			connections().remove(clientId(iter.value().address, iter.value().port)); // no longer send to it, client has to reconnect
		}
	}
}

void AudioServerThread::enqueueDatagram(const QByteArray &datagram)
{
	m_sendQueueMutex.lock();
	m_sendQueue.enqueue(datagram);
	m_sendQueueMutex.unlock();
}

void AudioServerThread::sendQueuedDatagrams()
{
	if ( m_sendQueueMutex.tryLock(0) ) {
		while ( !m_sendQueue.isEmpty() )
			sendDatagram(m_sendQueue.dequeue());
		m_sendQueueMutex.unlock();
	}
}

void AudioServerThread::run()
{
	setSocket(new QUdpSocket);
	if ( bindToLocalPort() ) {
		while ( !m_exit ) {
			readPendingDatagrams();
			sendQueuedDatagrams();
			usleep(5000);
		}
	}
	delete socket();
}
