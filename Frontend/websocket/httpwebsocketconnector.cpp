#include "httpwebsocketconnector.h"
#include <QDebug>
namespace REST {

WebsocketQHttpServerAdapter::WebsocketQHttpServerAdapter(QAbstractHttpServer *httpsrv)
    : QObject{httpsrv}, mServer(httpsrv)
{
    qDebug()<<"create web socket adapter";
    if (mServer)
    {
        connect(mServer, &QAbstractHttpServer::newWebSocketConnection, this , &WebsocketQHttpServerAdapter::onNewWebSocketConnection);
    }
}

void WebsocketQHttpServerAdapter::onNewWebSocketConnection()
{
    while(mServer->hasPendingWebSocketConnections())
    {
        auto ws = mServer->nextPendingWebSocketConnection();
        qDebug()<<"WebSocket conn from"<<ws->peerName()<<"with path"<<ws->requestUrl();

        ws->sendTextMessage("HELO");
        ws->deleteLater();
    }

}
}
