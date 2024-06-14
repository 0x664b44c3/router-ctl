#include "panelproxy.h"
#include <QTimer>
#include <QByteArray>
#include <QTcpSocket>
#include <QString>
#include <QStringList>
#include "panelserver.h"
#include <iostream>
#include <QDebug>
#include <QDateTime>
#include <QTextStream>
namespace BMD {

static QTextStream console(stdout);
PanelProxy::PanelProxy(QTcpSocket *sock, PanelServer *parent)
    : mSock(sock), mRouterConnection(new QTcpSocket), QObject{parent}
{
    connect(mSock,
            &QTcpSocket::readyRead, this, &PanelProxy::onDataReadyPanel);
    connect(mRouterConnection, &QTcpSocket::connected, this, []() { qDebug()<<"router connection established";});
    connect(mRouterConnection,
            &QTcpSocket::readyRead, this, &PanelProxy::onDataReadyServer);
    mRouterConnection->connectToHost("192.168.0.34", 9990);
    qDebug()<<"panel proxy created";
}

void PanelProxy::checkFullPackets()
{
    int o=0;
    int p=mBufferP2H.indexOf("\n", o);
    while(p>=0)
    {
        QByteArray line = mBufferP2H.mid(o, p-o);
        debugPrint("", line);
        line.append('\n');
        mRouterConnection->write(line);
        mRouterConnection->flush();
        o=p+1;
        p=mBufferP2H.indexOf("\n", o);
    }
    mBufferP2H.remove(0,o);
    o=0;
    p=mBufferH2P.indexOf("\n", o);
    while(p>=0)
    {
        QByteArray line = mBufferH2P.mid(o, p-o);
        debugPrint("", line, true);
        line.append('\n');
        mSock->write(line);
        mSock->flush();
        o=p+1;
        p=mBufferH2P.indexOf("\n", o);
    }
    mBufferH2P.remove(0,o);
}

void PanelProxy::debugPrint(QString tag, const QByteArray &d, bool indent)
{
    QString shift;
    shift.fill(' ', indent?40:0);
    console << QTime::currentTime().toString("hh:mm:ss.zzz ") << shift << tag<< "\""<<d<<"\"\n";
    console.flush();
}


void PanelProxy::onDisconnect()
{
    deleteLater();
}

void PanelProxy::onDataReadyPanel()
{
    while (!mSock->atEnd())
    {
        QByteArray data = mSock->readAll();
        // mRouterConnection->write(data);
        // mRouterConnection->flush();
        mBufferP2H.append(data);
        checkFullPackets();
    }
}

void PanelProxy::onDataReadyServer()
{
    // qDebug()<<"read data from video hub";
    while (!mRouterConnection->atEnd())
    {
        QByteArray data = mRouterConnection->readAll();
        // mSock->write(data);
        // mSock->flush();
        mBufferH2P.append(data);
        checkFullPackets();
    }
}

} // namespace BMD

