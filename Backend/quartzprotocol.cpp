#include "quartzprotocol.h"
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

#define DEFAULT_BAUD 38400

#define DRIVER_NAME "QUARTZ"

#define MAX_LEVEL 7
static QString LEVELS("VABCDEFGHIJKLMNO");

namespace Router {

QuartzProtocolDriver::QuartzProtocolDriver(QString busId, QObject * parent) :
    IOStreamBusDriverBase(busId, parent)
{
    auto t = new QTimer(this);
    t->setInterval(1000);
    t->start();
    connect(t, &QTimer::timeout, this, &QuartzProtocolDriver::onTimer);
    mDriverName = DRIVER_NAME;
}

bool QuartzProtocolDriver::rescanBus()
{
    for(int level=0;level<16;++level)
        queryRouter(0, 0);
    return true;
}

QString QuartzProtocolDriver::driverName() const
{
    return QStringLiteral("Evertz/Quartz");
}

QString QuartzProtocolDriver::driverInfo() const
{
    return QStringLiteral(
        "Driver for older Evertz Quartz series routers.\n"
        );
}

void QuartzProtocolDriver::queryRouter(int addr, int level)
{
    Q_UNUSED(addr);
    level = std::clamp(level, 0, MAX_LEVEL);
    for(int offset=0; offset<64; offset+=8)
        mCommandQueue.append(".L" +
                             LEVELS.mid(level,1) +
                             QString::asprintf("%d,-", offset+1));
    checkQueue();
}

void QuartzProtocolDriver::queryAlarms(int addr, int level)
{
    Q_UNUSED(addr)
    Q_UNUSED(level)
}


void QuartzProtocolDriver::setXPoint(int addr, int level, int dest, int source)
{
    Q_UNUSED(addr);
    level = std::clamp(level, 0, MAX_LEVEL);
    QString cmd = ".S" + LEVELS.mid(level,1) + QString::asprintf("%d,%d\r", dest+1, source+1);
    mCommandQueue.append(cmd);
    checkQueue();
}

void QuartzProtocolDriver::checkQueue()
{
    if (!mChannel)
        return;
    if (!mHasResponse)
        return;
    if(mCommandQueue.isEmpty())
        return;
    QString cmd = mCommandQueue.takeFirst();
    mLastCommand = cmd;
    QByteArray d = cmd.toLatin1();
    d.append("\r");
    // qDebug()<<"Cmd: "<<cmd;
    mChannel->write(d);
    mHasResponse = false;
    mTimeSinceCmd.restart();
}

void QuartzProtocolDriver::processRx(const QByteArray & d)
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
            mHasResponse = true;
            mLogonStage = LogonStage::LoggedIn;
        } else if (currentLine.contains("login:") && ((mLogonStage == LogonStage::Connected) || (mLogonStage == LogonStage::PassSent))) {
            if (mChannel)
            {
                mLogonStage = LogonStage::UserSent;
                mChannel->write(mUser.toLatin1());
            }
        } else if (currentLine.contains("pass") && (mLogonStage == LogonStage::UserSent)) {
            if (mChannel)
            {
                mLogonStage = LogonStage::PassSent;
                mChannel->write(mPass.toLatin1());
            }
        }
    }
    if (mHasResponse)
    {
        checkQueue();
    }
}


void QuartzProtocolDriver::parseLine(const QByteArray &l)
{
    mHasResponse = true;
    QString line = QString::fromLatin1(l);
    mLastResponse = line;
    if(line.startsWith("."))
        mLogonStage = LogonStage::LoggedIn;

    if (l==".E")
    {
        qDebug()<<"Quartz Rotuter returned an error for cmd"<<mLastCommand;
        return;
    }
    if (l==".A")
    {
        return;
    }
    if (l==".#01")
    {
        // mCommandQueue.append(".A");
        // checkQueue();
        return;
    }
    // qDebug()<<line;

    if (line.size()<3)
        return;
    int ll = line.length();
    //.AV0,0
    //0123456
    if (line.startsWith('.'))
    {
        if (line.startsWith(".A") && line[2].isLetter())
        { // list routes response
            int i=2; // letter of the first level
            while(i<ll-3) //shortest possible route is [level][digit],[digit] (4 chars)
            {
                QString level = line.mid(i,1);
                QString dst;
                QString src;
                ++i; // skip over level
                int j = i;
                while(i<ll)
                {
                    if (line[i].isDigit())
                        ++i;
                    else
                        break;
                }
                dst = line.mid(j, i-j);
                if (i>=ll-3) // at least one comma and one digit must follow
                    break;
                if (line[i]!=',')
                    break;
                ++i; // skip over comma (checked for remaining length before)
                j = i;
                while(i<ll)
                {
                    if (line[i].isDigit())
                        ++i;
                    else
                        break;
                }
                src = line.mid(j, i-j);

                emit XPointChanged(mBusId, 0, LEVELS.indexOf(level), dst.toInt(nullptr, 10), src.toInt(nullptr, 10));
                // qDebug()<<"Quartz reported route on level"<<level<<"Dest"<<dst<<"has Source"<<src;
            }
        }
        if ((ll>2) && (line.startsWith(".U") || line.startsWith(".S")))
        {  //.U{levels}{dest},{srce}(cr)

            int i=2; // letter of the first level
            while(i<ll)
            {
                if (line[i].isLetter())
                    ++i;
                else
                    break;
            }
            QString levels = line.mid(2, i-2);

            QString dst;
            QString src;
            int j = i;
            while(i<ll)
            {
                if (line[i].isDigit())
                    ++i;
                else
                    break;
            }
            dst = line.mid(j, i-j);
            if (i>=ll-3) // at least one comma and one digit must follow
                return;
            if (line[i]!=',')
                return;
            ++i; // skip over comma (checked for remaining length before)
            j = i;
            while(i<ll)
            {
                if (line[i].isDigit())
                    ++i;
                else
                    return;
            }
            src = line.mid(j, i-j);
            for(int k = 0; k< levels.size();++k)
            {
                emit XPointChanged(mBusId, 0, LEVELS.indexOf(levels[k]), dst.toInt(nullptr, 10), src.toInt(nullptr, 10));
                // qDebug()<<"Quartz reported route on level"<<levels[k]<<"Dest"<<dst<<"has Source"<<src;
            }

        }
    }

    else
    {
        qDebug()<<"Unknown line format:"<<line;
    }


}


void QuartzProtocolDriver::onTimer()
{
    if (!mHasResponse && mTimeSinceCmd.elapsed() >= CMD_TIMEOUT)
    {
        if (!mChannel)
            return;
        if (!mChannel->isOpen())
            return; // silence the warning messages about disconnected sockets and closed comm port
        if ((mLogonStage!=LogonStage::LoggedIn) && (mTimeSinceCmd.elapsed() < LOGIN_TIMEOUT))
            return;
        mChannel->write(".#01\r");
        mTimeSinceCmd.restart();
        return;
    }
    if (mHasResponse &&  mTimeSinceCmd.elapsed() >= IDLE_TIMEOUT)
    {
        //keep connection active to avoid disconnection b/c timeout
        mCommandQueue << ".#01";
    }

    if (mCommandQueue.size())
    {
        checkQueue();
    }
}


void QuartzProtocolDriver::onChannelConnected()
{
    mCommandQueue.clear();
    mCommandQueue << ".#01";
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
            mChannel->write(".#01\r");
        mLogonStage = LogonStage::LoggedIn;
        break;
    }
}

void QuartzProtocolDriver::onChannelDisconnected()
{

}


} //namespace

