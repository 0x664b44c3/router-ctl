#include "bmd_videohub_driver.h"
#include <QIODevice>
#include <QDebug>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QTcpSocket>

#ifdef QT_SERIALPORT_LIB
#include <QSerialPort>
#define WITH_SERIAL_PORT
#endif

#ifdef QT_NETWORK_LIB
#include <QTcpSocket>
#define WITH_SERIAL_PORT
#endif

#define DRIVER_NAME "BMD-VIDEOHUB"
#define IDLE_TIMEOUT 2

namespace Router {

BmdBusDriver::BmdBusDriver(QString busId, QObject * parent) :
    IOStreamBusDriverBase(busId, parent, 9990)
{
    auto t = new QTimer(this);
    t->setInterval(1000);
    t->start();
    connect(t, &QTimer::timeout, this, &BmdBusDriver::onTimer);
    mDriverName = DRIVER_NAME;
    mCurrentRun = 0;
}

bool BmdBusDriver::rescanBus()
{
    // for(int level=0;level<16;++level)
    //     queryRouter(0, level);
    return true;
}

QString BmdBusDriver::driverName() const
{
    return QString("BMD/Videohub [INCOMPLETE]");
}

QString BmdBusDriver::driverInfo() const
{
    return QString("incomplete driver for BMD videohub");
}

void BmdBusDriver::queryRouter(int addr, int level)
{
    Q_UNUSED(addr);
    //is there a way to query all xpoints?
    // mCommandQueue << QString::asprintf("@ S?%x", level & 15);
    checkQueue();
}

void BmdBusDriver::queryAlarms(int addr, int level)
{

}

bool BmdBusDriver::checkBufferForAck() const
{
    return true;
    // return mBuffer.endsWith(">") || mBuffer.endsWith("> ") ;
}

void BmdBusDriver::setXPoint(int addr, int level, int dest, int source)
{
    Q_UNUSED(addr)  //BMD is always a point-to-point link to one router
    Q_UNUSED(level) //BMD videohub has no levels
    // if (source==-1) // is unpatch even supported at all?
    // {
    //     // mCommandQueue.append(
    //     //     QString::asprintf("@ X:%d/%x,X", level, dest));
    // }
    // else
    // {
        mCommandQueue.append(
            QString::asprintf("VIDEO OUTPUT ROUTING:\n%d %d\n\n", dest, source));
    // }
    checkQueue();
}

void BmdBusDriver::checkQueue()
{
    // if (!mChannel)
    //     return;
    // if (!mHasPrompt)
    //     return;
    if(mCommandQueue.isEmpty())
        return;
    QByteArray d = mCommandQueue.takeFirst().toLatin1();
    // d.append("\n");
    qDebug()<<d;
    mChannel->write(d);
    mHasPrompt = false;
    mTimeSinceCmd.restart();
}

void BmdBusDriver::processRx(const QByteArray & d)
{
    checkQueue();
}


void BmdBusDriver::parseLine(const QByteArray &l)
{
}


void BmdBusDriver::onTimer()
{
    if (mHasPrompt &&  mTimeSinceCmd.elapsed() >= IDLE_TIMEOUT)
    {
        //keep connection active to avoid disconnection b/c timeout
        //(and also re-enable status reporting in case it got disabled along the way)
        mCommandQueue << "PING:\n\n";
    }
    if (mCommandQueue.size())
    {
        checkQueue();
    }
}


void BmdBusDriver::onChannelConnected()
{
    mCommandQueue.clear();

    QTimer * tmr = new QTimer(this);
    tmr->start(100);
    connect(tmr, &QTimer::timeout, this, [&]() {
        setXPoint(0,0,3,mCurrentRun);
        ++mCurrentRun;
        if (mCurrentRun>=20)
            mCurrentRun=0;
    });
    // mSock->wr
}

void BmdBusDriver::onChannelDisconnected()
{

}


} //namespace

