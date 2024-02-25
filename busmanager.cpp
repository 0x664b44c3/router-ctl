#include "busmanager.h"
#include "routercore.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "leitchbusdriver.h"


Router::BusManager * Router::BusManager::mInst = nullptr;
namespace Router {

BusManager::BusManager(QObject *parent)
    : QObject{parent}
{
    qInfo()<< "Created bus manager object.";
    qDebug()<<"Load upstream (matrix) bus drivers";
}

BusManager *BusManager::inst()
{
    return mInst;
}

bool BusManager::load(const QJsonObject & busses)
{
    for (auto it = busses.begin(); it!=busses.end(); ++it)
    {
        QJsonObject busDef = it.value().toObject();
        QString driver = busDef.value("driver").toString();
        AbstractBusDriver * bus;
        if (driver=="leitch")
        {
            bus = new LeitchBusDriver(this);
            bus->setDeviceUri(busDef.value("url").toString());
            qDebug()<<"Created bus of type"<<driver;
            bus->connectBus();
        }
        else
        {
            qWarning()<<"Driver"<<driver<<"not found";
            continue;
        }

    }

    return false;
}

} // namespace Router
