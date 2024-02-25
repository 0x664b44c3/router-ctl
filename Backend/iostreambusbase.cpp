#include "iostreambusbase.h"
#include <QIODevice>
#include <QDebug>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <telnetclient.h>
#include <QTcpSocket>

#ifdef QT_SERIALPORT_LIB
#include <QSerialPort>
#define WITH_SERIAL_PORT
#endif

#ifdef QT_NETWORK_LIB
#include <QTcpSocket>
#define WITH_SERIAL_PORT
#endif

#define DEFAULT_BAUD 9600


namespace Router {

IOStreamBusDriverBase::IOStreamBusDriverBase(QString busId, QObject * parent) :
    AbstractBusDriver(busId, parent),
    mChannel(nullptr),
    mTelnet(nullptr),
    mDriverName("generic-stream"),
    mConnectionType(connNone)
{

}

bool IOStreamBusDriverBase::connectBus()
{
    if (mChannel)
    {
        mChannel->deleteLater();
        mChannel = nullptr;
    }
    if (mTelnet)
    {
        mTelnet->deleteLater();
        mTelnet = nullptr;
    }
    /* TODO: add bus uri parsing and do stuff here */
    QUrl url(mDeviceUri);
    QString scheme = url.scheme().toLower();
    if (scheme == "serial")
    {
        mConnectionType = connSerial;
        auto port = new QSerialPort(this);
        QString devPath = url.host() + url.path();
        port->setPortName(devPath);
        int baud = DEFAULT_BAUD; //defualt for leitch panacea
        if (url.hasQuery())
        {
            QUrlQuery query(url);
            QString baudString = query.queryItemValue("baud");
            if (!baudString.isEmpty())
            {
                bool ok = false;
                baud = baudString.toUInt(&ok);
                if (!ok)
                {
                    qWarning()<<"Baud argument wais not numeric:"<<baudString;
                    baud = 9600;
                }
            }
            port->setBaudRate(baud);
            if (port->open(QIODevice::ReadWrite))
            {
                qInfo()<<"Serial port"<<port->portName()<<"opened OK";
            }
            else
            {
                qCritical()<<"Could not open serial port"<<port->portName()<<port->errorString();
            }
            if (!port->isOpen())
                port = nullptr;
            mChannel = port;
        }
    }
    else if ((scheme == "tcp") || (scheme == "socket") || (scheme=="telnet"))
    {
        QTcpSocket *socket = nullptr;
        QHostAddress host = QHostAddress(url.host());
        quint16      port = url.port(23);
        bool useTelnet = (scheme == "telnet");
        if (useTelnet)
        {
            mConnectionType = connTelnet;
            mTelnet = new TelnetClient(this);
            mTelnet->connectToHost(host, port);

            connect(mTelnet, &TelnetClient::readyRead, this, [&](){
                while(!mTelnet->atEnd())
                {
                    this->processRx(mTelnet->getData());
                }});
            socket = mTelnet->ioDev();
        }
        else
        {
            mConnectionType = connTCP;
            socket = new QTcpSocket(this);
            socket->connectToHost(host, port);
            connect(socket, &QTcpSocket::readyRead, this, [&](){
                while(!mChannel->atEnd())
                {
                    this->processRx(mChannel->readAll());
                }
            });
        }
        connect(socket, &QTcpSocket::connected, this, &IOStreamBusDriverBase::onChannelConnected);
        setChannel(socket);
        socket->state();

    }
    else
    {
        qDebug().noquote()<<mDriverName<<": unknown url scheme" << url.scheme();
        qDebug()<<"Device URL:"<<mDeviceUri;
        return false;
    }
    return mChannel?true:false;
}

void IOStreamBusDriverBase::disconnectBus()
{
    //ensures we do not auto-reopen the connection immediately
    mConnectionType = connNone;

    if (mTelnet)
        mTelnet->disconnect();

    if (mChannel)
        mChannel->close();
}

bool IOStreamBusDriverBase::isOnline() const
{
    if (!mChannel)
        return false;
    if (!mChannel->isOpen())
        return false;
    if ((mConnectionType == connTCP) || (mConnectionType == connTelnet))
    {
        QTcpSocket * sock = qobject_cast<QTcpSocket*>(mChannel);
        if ((sock) && (sock->state() != QAbstractSocket::ConnectedState))
            return false;
    }
    return mChannel->isWritable();
}

void IOStreamBusDriverBase::setChannel(QIODevice *iodev)
{
    mChannel = iodev;
}


void IOStreamBusDriverBase::onReadyRead()
{
    if (!mChannel)
        return;
    while(!mChannel->atEnd())
        processRx(mChannel->readAll());
}

}

