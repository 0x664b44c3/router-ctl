QT += core network serialport
QT += gui widgets

CONFIG += c++17 c++20
QMAKE_CXXFLAGS += -std=c++20 -std=gnu++20

TARGET = router_ctl_testbed
CONFIG -= app_bundle
#CONFIG +=console

DEFINES+= DISABLE_DBG

TEMPLATE = app

SOURCES += main.cpp \
    umdcontroller.cpp

HEADERS += \
    objectfactory.h \
    umdcontroller.h

DISTFILES += \
	 remote_control_protocol.txt \
    config.txt


include($$PWD/RouterEngine/router_engine.pri)
include($$PWD/Backend/backend.pri)
include($$PWD/Telnet/telnet.pri)

include($$PWD/ControlServer/control_server.pri)

include($$PWD/Frontend/frontend.pri)



RESOURCES +=

STATECHARTS +=
