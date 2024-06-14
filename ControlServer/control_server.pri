INCLUDEPATH +=$$PWD

QT += core network serialport
CONFIG += c++11

INCLUDEPATH+=$$PWD

HEADERS += \
    $$PWD/panelconnection.h \
    $$PWD/panelproxy.h \
    $$PWD/panelserver.h

SOURCES += \
    $$PWD/panelconnection.cpp \
    $$PWD/panelproxy.cpp \
    $$PWD/panelserver.cpp
