#-------------------------------------------------
#
# Project created by QtCreator 2011-10-04T20:47:27
#
#-------------------------------------------------
QT       += core gui
QT += sql
QT += network

TARGET = Sverver-V1
TEMPLATE = app


SOURCES += main.cpp\
        ServerMainDialog.cpp \
    ThreadTcpServer.cpp \
    ThreadTcpSocket.cpp

HEADERS  += ServerMainDialog.h \
    ThreadTcpServer.h \
    ThreadTcpSocket.h

FORMS    += ServerMainDialog.ui




