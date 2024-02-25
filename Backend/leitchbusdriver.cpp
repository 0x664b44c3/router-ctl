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
#define LOGIN_TIMEOUT 2000

#define DEFAULT_BAUD 9600

#define DRIVER_NAME "LEITCH"

namespace Router {

LeitchBusDriver::LeitchBusDriver(QString busId, QObject * parent) :
    IOStreamBusDriverBase(busId, parent)
{
    auto t = new QTimer(this);
    t->setInterval(1000);
    t->start();
    connect(t, &QTimer::timeout, this, &LeitchBusDriver::onTimer);
    mDriverName = DRIVER_NAME;
}

bool LeitchBusDriver::rescanBus()
{
    for(int level=0;level<16;++level)
        queryRouter(0, level);
    return true;
}

void LeitchBusDriver::queryRouter(int addr, int level)
{
    Q_UNUSED(addr);
    mCommandQueue << QString::asprintf("@ S?%x", level & 15);
    checkQueue();
}

void LeitchBusDriver::queryAlarms(int addr, int level)
{

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

        //while waiting for Login, reset timer on each logon banner line
        if (mLogonStage != LogonStage::LoggedIn)
            mTimeSinceCmd.restart();
    }
    mBuffer = mBuffer.mid(offset);
    offset=0;
    if (mBuffer.size()) {
        QByteArray currentLine = mBuffer.mid(offset);
        if (currentLine.contains('>'))
        {
            mHasPrompt = true;
            mLogonStage = LogonStage::LoggedIn;
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
                emit XPointChanged(mBusId,
                                   0, level, dst, src);
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
            int level = args.takeFirst().toInt(nullptr, 16); // level (?)
            args.takeFirst(); // unknown
            int src = args.takeFirst().toUInt(nullptr, 16);
            args.takeFirst(); // unknown
            bool active = args.takeFirst().toUInt(nullptr, 16)?true:false;
            if (mDebug)
                qDebug().nospace()<<mBusId << " Source " <<  level <<"/"<< src << (active?" active":" inactive");
            emit sourceActive(mBusId,
                              0, level, src, active);
        }

    }
    else
    {
        if(mDebug)
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
            return; // silence the warning messages about disconnected sockets and closed comm port
        if ((mLogonStage!=LogonStage::LoggedIn) && (mTimeSinceCmd.elapsed() < LOGIN_TIMEOUT))
            return;
        mChannel->write("\r\n");
        mTimeSinceCmd.restart();
        return;
    }
    if (mHasPrompt &&  mTimeSinceCmd.elapsed() >= IDLE_TIMEOUT)
    {
        //keep connection active to avoid disconnection b/c timeout
        //(and also re-enable status reporting in case it got disabled along the way)
        mCommandQueue << "@ ?";
    }
    if (mCommandQueue.size())
    {
        checkQueue();
    }
}


void LeitchBusDriver::onChannelConnected()
{
    mCommandQueue.clear();
    mCommandQueue << "@ ?";
    mCommandQueue << "@ ?";
    mCommandQueue << "TERMINAL OFF";
    rescanBus();
    mTimeSinceCmd.start();
    qDebug()<<"Connected"<<mBusId;
    mLogonStage = LogonStage::Connected;

    switch(mConnectionType)
    {
    case connTelnet:
        break; // we will wait for timeout or user prompt

    case connTCP:
    case connSerial: //direct connections have no logon, skip logon detection, try to get a prompt directly
        if (mChannel)
            mChannel->write("\r");
        mLogonStage = LogonStage::LoggedIn;
        break;
    }
}

void LeitchBusDriver::onChannelDisconnected()
{

}


} //namespace

