INCLUDEPATH+=$$PWD

#ensure we have the network module
QT+=network


# pull in FastCGI module
include($$PWD/../submodules/fastcgi/fastcgi/fastcgi.pri)
include($$PWD/REST/rest-frontend.pri)

# include a REST server based on QHttpServer module
include($$PWD/httpserver/httpserver.pri)


qtHaveModule(websockets) {
    include($$PWD/websocket/websocket.pri)
}

HEADERS += \
    $$PWD/../submodules/fastcgi/fcgiapp.h

SOURCES += \
    $$PWD/../submodules/fastcgi/fcgiapp.cpp


#webui ressources
RESOURCES += \
    $$PWD/webui.qrc

DISTFILES += \
    $$PWD/webui/error_page.html \
    $$PWD/webui/index.html \
    $$PWD/webui/web_panel.html
