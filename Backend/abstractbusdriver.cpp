#include "abstractbusdriver.h"
namespace Router {
AbstractBusDriver::AbstractBusDriver(QString busId, QObject *parent)
    : QObject{parent}, mBusId(busId), mDebug(false)
{

}

QString AbstractBusDriver::deviceUri() const
{
    return mDeviceUri;
}

void AbstractBusDriver::setDeviceUri(const QString &newDeviceUri)
{
    mDeviceUri = newDeviceUri;
}

bool AbstractBusDriver::debug() const
{
    return mDebug;
}

void AbstractBusDriver::setDebug(bool newDebug)
{
    mDebug = newDebug;
}

const QString &AbstractBusDriver::busId() const
{
    return mBusId;
}

void AbstractBusDriver::setBusId(const QString &newBusId)
{
    mBusId = newBusId;
}

QString AbstractBusDriver::driverName() const
{
    return QStringLiteral("abstract bus i/f");
}

QString AbstractBusDriver::driverInfo() const
{
    return QStringLiteral("abstract base driver class");
}
}
