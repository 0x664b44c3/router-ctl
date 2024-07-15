# this implements a minimalistic Telnet "client" (more like a filter) that basially only strips off any telnet attributes
QT += network
CONFIG += c++11

INCLUDEPATH+=$$PWD
SOURCES +=\
    $$PWD/telnetclient.cpp
HEADERS += \
    $$PWD/telnetclient.h \
    $$PWD/telnet-options.h
