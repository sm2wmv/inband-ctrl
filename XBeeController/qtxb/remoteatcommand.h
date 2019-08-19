#ifndef REMOTEATCOMMAND_H
#define REMOTEATCOMMAND_H

#include "digimeshpacket.h"
#include <QByteArray>

class RemoteATCommand : public DigiMeshPacket
{
    QByteArray atCommand;
    QByteArray parameter;
public:
    explicit RemoteATCommand(QObject *parent);
    void setATCommand(QString command);
    void setParameter(QByteArray array);
    QByteArray getATCommand();
    QByteArray getParameter();
    void assemblePacket();
      
};

#endif // REMOTEATCOMMANDPACKET_H
