#include "remotecommandrequest.h"

RemoteCommandRequest::RemoteCommandRequest(QObject *parent) :
    DigiMeshPacket(parent)
{
    destAddr16.append(0xFF);
    destAddr16.append(0xFE);

    setFrameType(0x17);
    setFrameId(0x01);
}

void RemoteCommandRequest::setDestAddr64(QByteArray da64){
    destAddr64.clear();
    destAddr64.append(da64);
}

void RemoteCommandRequest::setDestAddr16(QByteArray da16){
    destAddr16.clear();
    destAddr16.append(da16);
}

QByteArray RemoteCommandRequest::getDestAddr64(){
    return destAddr64;
}

QByteArray RemoteCommandRequest::getDestAddr16(){
    return destAddr16;
}

void RemoteCommandRequest::setATCommand(QString command){
    atCommand.clear();
    atCommand.append(command.at(0));
    atCommand.append(command.at(1));
}

void RemoteCommandRequest::setParameter(QByteArray array){
    parameter.clear();
    parameter.append(array);
}

void RemoteCommandRequest::setRemoteCmdOptions(QByteArray options) {
    atCmdOptions = options;
}


QByteArray RemoteCommandRequest::getRemoteCmdOptions() {
    return atCmdOptions;
}

QByteArray RemoteCommandRequest::getATCommand(){
    return atCommand;
}

QByteArray RemoteCommandRequest::getParameter(){
    return parameter;
}

void RemoteCommandRequest::assemblePacket(){
    packet.clear();
    packet.append(getFrameType());
    packet.append(getFrameId());
    packet.append(getDestAddr64());
    packet.append(getDestAddr16());
    packet.append(getRemoteCmdOptions());
    packet.append(getATCommand());
    packet.append(getParameter());
    setLength(packet.size());
    createChecksum(packet);
    packet.append(getChecksum());
    packet.insert(0, getStartDelimiter());
    packet.insert(1, getLength());
}

