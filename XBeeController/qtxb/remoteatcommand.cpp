#include "remoteatcommand.h"
#include "digimeshpacket.h"

RemoteATCommand::ATCommand(QObject *parent) :
    DigiMeshPacket(parent)
{
    setFrameType(0x08);
    setFrameId(0x00);
}


void RemoteATCommand::setATCommand(QString command){
    atCommand.clear();
    atCommand.append(command.at(0));
    atCommand.append(command.at(1));
}

void RemoteATCommand::setParameter(QByteArray array){
    parameter.clear();
    parameter.append(array);
}
QByteArray RemoteATCommand::getATCommand(){
    return atCommand;
}
QByteArray RemoteATCommand::getParameter(){
    return parameter;
}
void RemoteATCommand::assemblePacket(){
    packet.clear();
    packet.append(getFrameType());
    packet.append(getFrameId());
    packet.append(getATCommand());
    packet.append(getParameter());
    setLength(packet.size());
    createChecksum(packet);
    packet.append(getChecksum());
    packet.insert(0, getStartDelimiter());
    packet.insert(1, getLength());
}

