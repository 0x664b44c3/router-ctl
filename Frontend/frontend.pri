INCLUDEPATH+=$$PWD

#ensure we have the network module
QT+=network


# pull in FastCGI module
include($$PWD/../submodules/fastcgi/fastcgi/fastcgi.pri)
include($$PWD/REST/rest-frontend.pri)

# include a REST server based on QHttpServer module
qtHaveModule(httpserver) {
   include($$PWD/httpserver/httpserver.pri)
}

qtHaveModule(websockets) {
    include($$PWD/websocket/websocket.pri)
}
