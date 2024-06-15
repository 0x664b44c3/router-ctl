INCLUDEPATH+=$$PWD

HEADERS += \
    $$PWD/rest_types.h \
    $$PWD/restcontroller.h \
    $$PWD/restressourceinterface.h

SOURCES += \
    $$PWD/restcontroller.cpp \
    $$PWD/restressourceinterface.cpp


qtHaveModule(httpserver) {
HEADERS += \
    $$PWD/qhttpserveradapter.h
SOURCES += \
    $$PWD/qhttpserveradapter.cpp
}
