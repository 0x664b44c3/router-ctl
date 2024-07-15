#include "busmanager.h"
#include "routercore.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "leitchbusdriver.h"
#include "quartzprotocol.h"
#include "vikinx_protocol.h"
#include "bmd_videohub_driver.h"
#include "objectfactory.h"
#include <iostream>
#include <QTimer>

#include <QCoreApplication>

#define OFFLINE_RETRY 9000
Router::BusManager * Router::BusManager::mInst = nullptr;

namespace Router {

BusManager::BusManager(QObject *parent)
    : QObject{parent},mDriverFactory(new ObjectFactory<AbstractBusDriver>()),mAlarmState(0), mDebug(false)
{
    qInfo()<< "Created bus manager object.";

    QTimer * tmr = new QTimer(this);
    tmr->start(2500);
    connect(tmr, &QTimer::timeout, this, &BusManager::onTimer);

    qDebug()<<"Load upstream (matrix) bus drivers";
    FactoryImpl<AbstractBusDriver, LeitchBusDriver>::registerToFactory(mDriverFactory, "leitch");
    FactoryImpl<AbstractBusDriver, QuartzProtocolDriver>::registerToFactory(mDriverFactory, "quartz");
    FactoryImpl<AbstractBusDriver, VikinxProtocolDriver>::registerToFactory(mDriverFactory, "vikinx");
    // mDriverFactory->registerClass("gvg-ascii", nullptr);
    FactoryImpl<AbstractBusDriver, BmdBusDriver>::registerToFactory(mDriverFactory, "videohub");
    mDriverFactory->registerClass("network", nullptr);
}

int BusManager::alarms() const
{
    return mAlarmState;
}

int BusManager::alarmsForBus(QString busId) const
{
    return 0;
}

void BusManager::debugBusState() const
{
    qDebug()<<" ALM  Started  Online  Driver:BusId";
    for (auto it = mBusses.begin(); it != mBusses.end(); ++it)
    {
        AbstractBusDriver * bus = it->bus;
        int alarms=it->alarm;
        bool online=false;
        if (bus)
        {
            online = bus->isOnline();
            if (!online)
                alarms|=alarmBusOffline;
        }
        QString msg = QString::asprintf("%04x  ",alarms)
                      +(it->wasStarted?"  yes  ":"   no  ") + "  "
                      + (online?" true ":" false") + "  "
                      + it->driver +":"+ it.key();
        qDebug().noquote()<<msg;
    }
}

AbstractBusDriver *BusManager::bus(QString id) const
{
    if (!mBusses.contains(id))
        return nullptr;
    return mBusses.value(id).bus;
}

QStringList BusManager::getBusIds() const
{
    return mBusses.keys();
}

void BusManager::checkBusses()
{

    int alm=0;
    for (auto it = mBusses.begin(); it != mBusses.end(); ++it)
    {
        alm |= it->alarm;
        AbstractBusDriver *bus = it->bus;
        if (bus)
        {
            if (!bus->isOnline()) {
                alm|=alarmBusOffline;
                if (it->offlineTimer.elapsed() > OFFLINE_RETRY)
                {
                    it->offlineTimer.restart();
                    if(bus->connectBus())
                        it->alarm &= ~alarmConnectFailed;
                    else
                        it->alarm |=  alarmConnectFailed;
                }
            }
            else
            {
                it->offlineTimer.restart();
            }
        }
        else
            alm |= alarmNoSuchDriver;
    }
    mAlarmState = alm;
}

void BusManager::onTimer()
{
    static int previousAlarms=0xffff;
    bool showBusState = false;

    checkBusses();

    return;
    int alms = alarms();
    if (alms)
        showBusState = true;
    if (previousAlarms)
        showBusState = true;

    if (showBusState)
    {
        if (alms)
        {
            qWarning()<<"Bus Alarms present";
            std::cerr << "\a" << std::flush;
        }
        debugBusState();
        // }
        // QJsonObject
    }
    if ((previousAlarms) && (alms == 0))
        qDebug()<<"All previous bus alarms were deasserted.";
    previousAlarms = alms;

}

void BusManager::onMatrixFound(QString busId, int addr, int level, int nSrc, int nDst, int src_offset, int dst_offset)
{

}

void BusManager::onXPointChanged(QString busId, int addr, int level, int dst, int src)
{

}

void BusManager::onSourceActive(QString busId, int addr, int level, int src, bool active)
{

}

void BusManager::onDrvConnected(QString busId)
{

}

void BusManager::onDrvConnectionLost(QString busId)
{

}

BusManager *BusManager::inst()
{
    if(!mInst)
        mInst = new BusManager(QCoreApplication::instance());
    return mInst;
}

bool BusManager::load(const QJsonObject & busses)
{
    for (auto it = busses.begin(); it!=busses.end(); ++it)
    {
        QJsonObject busDef = it.value().toObject();
        QString busId = it.key();
        if (mBusses.contains(busId))
        {
            qWarning()<<"Bus id"<<busId<<"already in use.";
            continue;
        }

        QString driver = busDef.value("driver").toString();
        QString url    = busDef.value("url").toString();

        BusEntry bus;
        bus.driver     = driver;
        bus.url        = url;
        bus.wasStarted = false;
        bus.alarm      = 0;
        bus.offlineTimer.start();
        if (busDef.value("options").isObject())
        {
            bus.parameters = busDef.value("options").toObject().toVariantMap();
        }
        bus.bus = nullptr;

        bus.bus = mDriverFactory->create(bus.driver,
                                         busId,
                                         this);
        if (!bus.bus)
        {
            qWarning()<<"Driver"<<driver<<"not found";
            bus.alarm = alarmNoSuchDriver;
            mBusses.insert(busId, bus);

            //FIXME: add a catchall virtual bus driver to allow everything else to work as normal
            continue;
        }
        else
        {
            if (mDebug)
                qDebug()<<"Created bus of type"<<driver;
            bus.bus->setDeviceUri(bus.url);
            connect(bus.bus, &AbstractBusDriver::connected, this, &BusManager::onDrvConnected);
            connect(bus.bus, &AbstractBusDriver::connectionLost, this, &BusManager::onDrvConnectionLost);
            connect(bus.bus, &AbstractBusDriver::sourceActive, this, &BusManager::onSourceActive);
            connect(bus.bus, &AbstractBusDriver::XPointChanged, this, &BusManager::onXPointChanged);
            connect(bus.bus, &AbstractBusDriver::MatrixFound, this, &BusManager::onMatrixFound);

            auto bi = mBusses.insert(busId, bus);
            if (bi->bus->connectBus())
                bi->wasStarted = true;
            else
            {
                qWarning() << "Bus"<<busId<<"would not start (connectbus returned false)";
                bi->alarm|=alarmConnectFailed;
            }
        }
    }

    return true;
}

} // namespace Router
