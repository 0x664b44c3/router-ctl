INCLUDEPATH+=$$PWD/interface
INCLUDEPATH+=$$PWD

HEADERS += \
    $$PWD/interface/rest_types.h \
    $$PWD/interface/restcontroller.h \
    $$PWD/interface/restressourceinterface.h

SOURCES += \
    $$PWD/interface/restcontroller.cpp \
    $$PWD/interface/restressourceinterface.cpp

HEADERS+=\
    $$PWD/routerressource.h
SOURCES+=\
    $$PWD/routerressource.cpp


qtHaveModule(httpserver) {
HEADERS += \
    $$PWD/interface/qhttpserveradapter.h
SOURCES += \
    $$PWD/interface/qhttpserveradapter.cpp
}
