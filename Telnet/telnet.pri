QT += network
CONFIG += c++11

INCLUDEPATH+=$$PWD
SOURCES +=\
    $$PWD/telnetclient.cpp
HEADERS += \
    $$PWD/telnetclient.h \
    $$PWD/telnet-options.h
