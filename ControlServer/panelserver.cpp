#include "panelserver.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include "panelconnection.h"
#include "panelproxy.h"
namespace BMD {

PanelServer::PanelServer(quint16 port, QObject *parent)
    : mTcpPort(port), QObject{parent}, mServer(new QTcpServer(this))
{
    mServer->listen(QHostAddress::Any, mTcpPort);
    connect(mServer, &QTcpServer::newConnection, this, &PanelServer::onNewConnection);
}

void PanelServer::onNewConnection()
{
    while(mServer->hasPendingConnections())
    {
        QTcpSocket * conn = mServer->nextPendingConnection();
        // auto pc = new PanelConnection(conn, this);
        auto pc = new PanelProxy(conn, this);
    }
}

} // namespace BMD
