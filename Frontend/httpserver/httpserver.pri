INCLUDEPATH+=$$PWD

HEADERS += \
    $$PWD/http_codes.h \
    $$PWD/mimedb.h \
    $$PWD/staticfileserver.h

SOURCES += \
    $$PWD/http_codes.cpp \
    $$PWD/mimedb.cpp \
    $$PWD/staticfileserver.cpp


qtHaveModule(websockets) {
QT+=websockets
}

qtHaveModule(httpserver) {

DEFINES+=WITH_QT_HTTP_SERVER
QT+=httpserver

HEADERS += \
    $$PWD/httpresponderinterface.h \
    $$PWD/httpserver.h

SOURCES += \
    $$PWD/httpresponderinterface.cpp \
    $$PWD/httpserver.cpp

}

