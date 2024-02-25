#include "abstractbusdriver.h"
namespace Router {
AbstractBusDriver::AbstractBusDriver(QObject *parent)
    : QObject{parent}, mDebug(false)
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
}
