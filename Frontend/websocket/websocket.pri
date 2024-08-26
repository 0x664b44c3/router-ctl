 QT+=core network websockets

INCLUDEPATH+=$$PWD/websocket

HEADERS += \
    $$PWD/websocket/busserver.h \
    $$PWD/websocketdispatcher.h

SOURCES += \
    $$PWD/websocket/busserver.cpp \
    $$PWD/websocketdispatcher.cpp

qtHaveModule(httpserver) {
    HEADERS+= $$PWD/httpwebsocketconnector.h
    SOURCES+= $$PWD/httpwebsocketconnector.cpp \
}
