#ifndef REMOTECOMMANDREQUEST_H
#define REMOTECOMMANDREQUEST_H

#include "digimeshpacket.h"
#include <QByteArray>
#include <QObject>

class RemoteCommandRequest : public DigiMeshPacket
{
    QByteArray atCommand;
    QByteArray parameter;
    QByteArray destAddr64;
    QByteArray destAddr16;
    QByteArray atCmdOptions;
    public:
    explicit RemoteCommandRequest(QObject *parent);
    void setDestAddr64(QByteArray da64);
    void setDestAddr16(QByteArray da16);
    QByteArray getDestAddr64();
    QByteArray getDestAddr16();
    void setRemoteCmdOptions(QByteArray options);
    QByteArray getRemoteCmdOptions();
    void setATCommand(QString command);
    void setParameter(QByteArray array);
    QByteArray getATCommand();
    QByteArray getParameter();
    void assemblePacket();

};

#endif // REMOTECOMMANDREQUEST_H
