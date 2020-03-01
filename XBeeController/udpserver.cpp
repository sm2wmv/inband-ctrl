#include "udpserver.h"
#include <QXmlStreamReader>

UDPServer::UDPServer(QObject *parent) : QObject(parent)
{

}

void UDPServer::initSocket()
{
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress("192.168.1.81"), 12060);

    connect(udpSocket, SIGNAL(readyRead()),this, SLOT(readPendingDatagrams()));
}

void UDPServer::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);

        emit dataAvailable(datagram.data(), datagram.size(), &sender, &senderPort);
    }
}
