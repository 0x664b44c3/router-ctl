#include "leitchbusdriver.h"
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

#define CMD_TIMEOUT 1000
#define IDLE_TIMEOUT 10000

#define DEFAULT_BAUD 9600

#define DRIVER_NAME "LEITCH"
enum ConnectionType {
    connNone   = 0,
    connSerial = 1,
    connTCP    = 2,
    connTelnet = 3
};

namespace Router {

LeitchBusDriver::LeitchBusDriver(QObject * parent) :
    AbstractBusDriver(parent),
    mChannel(nullptr),
    mConnectionType(connNone),
    mTelnet(nullptr)
{
    auto t = new QTimer(this);
    t->setInterval(1000);
    t->start();
    connect(t, &QTimer::timeout, this, &LeitchBusDriver::onTimer);
    setDebug(true);
}

bool LeitchBusDriver::rescanBus()
{
    for(int level=0;level<16;++level)
        queryRouter(0, level);
    return true;
}

bool LeitchBusDriver::connectBus()
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
            mTelnet = new TelnetClient(this);
            mTelnet->connectToHost(host, port);

            connect(mTelnet, &TelnetClient::readyRead, this, [&](){
                while(!mTelnet->atEnd())
                {
                    this->processRx(mTelnet->getData());
                }});
            socket = mTelnet->ioDev();
        }
        connect(socket, &QTcpSocket::connected, this, &LeitchBusDriver::onChannelConncted);
        // connect(socket, &QTcpSocket::error, [&](QAbstractSocket::SocketError err) {
        //     qDebug()<< err<<qobject_cast<QTcpSocket>(mChannel).errorString();
        // });

        setChannel(socket);


    }
    else
    {
        qDebug()<<DRIVER_NAME<<": unknown url scheme" << url.scheme();
        qDebug()<<"Device URL:"<<mDeviceUri;
        return false;
    }
    return mChannel?true:false;
}

void LeitchBusDriver::disconnectBus()
{
    mConnectionType = connNone;
}

void LeitchBusDriver::queryRouter(int addr, int level)
{
    Q_UNUSED(addr);
    mCommandQueue << QString::asprintf("@ S?%x", level & 15);
    checkQueue();
}

bool LeitchBusDriver::checkBufferForPromt() const
{
    return mBuffer.endsWith(">") || mBuffer.endsWith("> ") ;
}

void LeitchBusDriver::setXPoint(int addr, int level, int dest, int source)
{
    Q_UNUSED(addr); //TODO: check with leitch spec if the frame address is used for anything

    if (source==-1) // unpatch output
        mCommandQueue.append(
            QString::asprintf("@ X:%d/%x,X", level, dest));
    else
        mCommandQueue.append(
            QString::asprintf("@ X:%d/%x,%x", level, dest, source));
    checkQueue();
}

void LeitchBusDriver::setChannel(QIODevice *iodev)
{
    // QObject::disconnect(this, SLOT(onReadyRead()));
    mChannel = iodev;
}

void LeitchBusDriver::checkQueue()
{
    if (!mChannel)
        return;
    if (!mHasPrompt)
        return;
    if(mCommandQueue.isEmpty())
        return;
    QByteArray d = mCommandQueue.takeFirst().toLatin1();
    d.append("\r");
    mChannel->write(d);
    mHasPrompt = false;
    mTimeSinceCmd.restart();
}

void LeitchBusDriver::processRx(const QByteArray & d)
{
    mBuffer.append(d);
    int offset = 0;
    mBuffer.replace('\n','\r');
    int endOfLine = mBuffer.indexOf("\r", offset);
    while(endOfLine>=0)
    {
        QByteArray line = mBuffer.mid(offset, endOfLine - offset).trimmed();
        if (line.size())
            parseLine(line);
        offset = endOfLine + 1;
        endOfLine = mBuffer.indexOf("\r", offset);
    }
    mBuffer = mBuffer.mid(offset);
    offset=0;
    if (mBuffer.size()) {
        QByteArray currentLine = mBuffer.mid(offset);
        if (currentLine.contains('>'))
        {
            mHasPrompt = true;
        } else if (currentLine.contains("login:") && ((mLogonStage == LogonStage::Connected) || (mLogonStage == LogonStage::PassSent))) {
            if (mChannel)
            {
                mLogonStage = LogonStage::UserSent;
                mChannel->write("leitch\r");
            }
        } else if (currentLine.contains("pass") && (mLogonStage == LogonStage::UserSent)) {
            if (mChannel)
            {
                mLogonStage = LogonStage::PassSent;
                mChannel->write("leitchadmin\r");
            }
        }
    }
    if (mHasPrompt)
    {
        checkQueue();
    }
}


void LeitchBusDriver::parseLine(const QByteArray &l)
{
    if (l.size()<3)
        return; // most likely just a prompt char
    //check if line is a valid report from the xbar
    QString line = QString::fromLatin1(l);
    while(line.startsWith(">"))
        line.remove(0,1);
    if (line.size()<3)
        return;
    if ((line[0].isLetterOrNumber()) && (line[1] == ':'))
    {
        // qDebug()<<"parse router msg:"<<line;
        if (line.startsWith("s:", Qt::CaseInsensitive))
        {
            int level = line.mid(2,1).toUInt(nullptr,16);
            // qDebug()<<"ignoring level info: "<<level;
            QString route = line.mid(3);
            QStringList args = route.split(',');
            while(args.size()>=2)
            {
                int dst  = args.takeFirst().toUInt(nullptr, 16);
                QString source = args.takeFirst();
                int src  = (source.toLower() == 'x')?-1:source.toUInt(nullptr, 16);
                if (mDebug)
                    qDebug()<<"On level"<<level<<"Dest "<<dst<<" is source "<<src;
                emit XPointChanged(0, level, dst, src);
            }
        }
    } else if (line.startsWith("E!s")) {
        //"E!s,0,0,B,1,1"
        //"E!s,0,0,6,1,0"
        // ___
        // input status
        //    <level?>,<?>,Input,<?>,status (1 == signal)
        QStringList args = line.split(',');
        if (args.size()>5)
        { // valid input status
            args.takeFirst(); // report code
            int level =0;
            args.takeFirst(); // level (?)
            args.takeFirst(); // unknown
            int src = args.takeFirst().toUInt(nullptr, 16);
            args.takeFirst(); // unknown
            bool active = args.takeFirst().toUInt(nullptr, 16)?true:false;
            qDebug()<<"Source" << src << (active?"active":"inactive");
            emit sourceActive(0, level, src, active);
        }

    }
    else
    {
        qDebug()<<"non status report line"<<l;
    }

}


void LeitchBusDriver::onTimer()
{
    if (!mHasPrompt && mTimeSinceCmd.elapsed() >= CMD_TIMEOUT)
    {
        if (!mChannel)
            return;
        if (!mChannel->isOpen())
            return; // silence the warning emssages about disconnected sockets and closed comm port
        mChannel->write("\r\n");
        return;
    }
    if (mHasPrompt &&  mTimeSinceCmd.elapsed() >= IDLE_TIMEOUT)
    {
        mCommandQueue.push_back("");
    }
    if (mCommandQueue.size())
    {
        checkQueue();
    }
}

void LeitchBusDriver::onReadyRead()
{
    if (!mChannel)
        return;
    while(!mChannel->atEnd())
        processRx(mChannel->readAll());
}

void LeitchBusDriver::onChannelConncted()
{
    mCommandQueue.clear();
    mCommandQueue << "@ ?";
    mCommandQueue << "@ ?";
    mCommandQueue << "TERMINAL OFF";
    mCommandQueue << "@ ?";
    rescanBus();
}







}

