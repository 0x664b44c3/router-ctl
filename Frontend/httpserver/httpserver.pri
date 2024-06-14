INCLUDEPATH+=$$PWD
QT+=httpserver

HEADERS += \
    $$PWD/http_codes.h \
    $$PWD/httpinterface.h

SOURCES += \
    $$PWD/http_codes.cpp \
    $$PWD/httpinterface.cpp

RESOURCES += \
    $$PWD/webui.qrc



