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
    defaultconfig.ini \
    config.txt \
    wwwdata/ui.html


include($$PWD/RouterEngine/router_engine.pri)
include($$PWD/Backend/backends.pri)
include($$PWD/Telnet/telnet.pri)

qtHaveModule(websockets) {
    include($$PWD/Frontend/proto/websocket.pri)
}


RESOURCES += \
    ressources.qrc

STATECHARTS +=
