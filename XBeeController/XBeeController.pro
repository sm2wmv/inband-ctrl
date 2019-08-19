#-------------------------------------------------
#
# Project created by QtCreator 2015-09-09T15:59:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = XBeeController
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \ 
    qtxb/txrequestexplicit.cpp \
    qtxb/txrequest.cpp \
    qtxb/transmitstatus.cpp \
    qtxb/rxindicatorexplicit.cpp \
    qtxb/rxindicator.cpp \
    qtxb/remotecommandresponse.cpp \
    qtxb/remotecommandrequest.cpp \
    qtxb/nodeidentificationindicator.cpp \
    qtxb/modemstatus.cpp \
    qtxb/digimeshpacket.cpp \
    qtxb/atcommandresponse.cpp \
    qtxb/atcommandremote.cpp \
    qtxb/atcommandqueueparam.cpp \
    qtxb/atcommand.cpp \
    qtxb.cpp

HEADERS  += mainwindow.h \
    qtxb/txrequestexplicit.h \
    qtxb/txrequest.h \
    qtxb/transmitstatus.h \
    qtxb/rxindicatorexplicit.h \
    qtxb/rxindicator.h \
    qtxb/remotecommandresponse.h \
    qtxb/remotecommandrequest.h \
    qtxb/nodeidentificationindicator.h \
    qtxb/modemstatus.h \
    qtxb/digimeshpacket.h \
    qtxb/atcommandresponse.h \
    qtxb/atcommandremote.h \
    qtxb/atcommandqueueparam.h \
    qtxb/atcommand.h \
    qtxb.h

FORMS    += mainwindow.ui
