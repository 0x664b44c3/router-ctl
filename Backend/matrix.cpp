#include "matrix.h"

#include <QJsonArray>
#include "busmanager.h"
#include <abstractbusdriver.h>
#include "json-utils.h"

namespace Router {

Matrix::PortInfo Matrix::portInfo(Port::Direction dir, int index) const
{
    if (index<0)
        return PortInfo();
    if (dir == Port::Direction::Source)
    {
        if (index>=mInputs.size())
            return PortInfo();
        return mInputs[index];
    }
    if (dir == Port::Direction::Destination)
    {
        if (index>=mOutputs.size())
            return PortInfo();
        return mOutputs[index];
    }
    return PortInfo();
}

int Matrix::numSources() const
{
    return mInputs.size();
}

int Matrix::numDestinations() const
{
    return mOutputs.size();
}

QJsonObject Matrix::serializePortInfo(const PortInfo &i)
{
    QJsonObject ret;
    //we don't store the ID in the config, use UID for it
    // ret.insert("id", i.id);
    ret.insert("uid", i.uid);
    ret.insert("label", i.label);
    ret.insert("mnemonic", i.mnemonic);
    return ret;
}

Matrix::PortInfo Matrix::deserialzePortInfo(const QJsonObject & obj, bool * ok)
{
    PortInfo info;
    if (ok)
    {
        if (obj.value("id").toString().isEmpty())
            *ok = false;
    }
    // info.id       = obj.value("id").toString();
    info.uid      = obj.value("uid").toString();
    info.mnemonic = obj.value("mnemonic").toString();
    info.label    = obj.value("label").toString();
    return info;
}

QJsonArray Matrix::getXPointsJson() const
{
    QJsonArray routing;
    for(auto i: std::as_const(mRouting))
        routing<<i;
    return routing;
}

int Matrix::getXPoint(int dest) const
{
    if ((dest<mFirstDest) || (dest>=(mFirstDest + numDestinations())))
        return -1;
    return mRouting.value(dest, -1);
}

Matrix::Matrix(QString id, QObject *parent)
    : QObject{parent}, mId(id),mFirstSource(0),mFirstDest(0),mAlarms(0)
{}

int Matrix::alarms() const
{
    int alm = mAlarms;
    auto bus = BusManager::inst()->bus(mBusId);
    if (!bus)
        alm|= almNoBus;
    else
    {
        if (!bus->isOnline())
            alm|=almBusOffline;
        else
            alm&=~almBusOffline;
    }
    return alm;
}

QStringList Matrix::alarmList() const
{
    QStringList alarmTexts;
    int alarm = alarms();
    if (alarm & almNoBus)
        alarmTexts << "[CRITICAL] No bus assigned";
    if (alarm & almBusOffline)
        alarmTexts << "[CRITICAL] Bus offline";
    alarm &= ~ 0x600;
    if (alarm) {
        for(int i=15; i>7; --i)
        {
            if (alarm & (1<<i))
                alarmTexts << "[CRITICAL] " + QString::asprintf("%04x", (1<<i));
        }
        for(int i=7; i>=0; --i)
        {
            if (alarm & (1<<i))
                alarmTexts << "[WARNING]  " + QString::asprintf("%04x", (1<<i));
        }
    }
    alarmTexts << BusManager::inst()->alarmStrings(mBusId);
    return alarmTexts;
}

bool Matrix::setBusAddress(QString bus, int level, int frame)
{
    mBusId   = bus;
    mLevel   = level;
    mBusAddr = frame;
    return true;
}

int Matrix::level() const
{
    return mLevel;
}

void Matrix::setLevel(int newLevel)
{
    mLevel = newLevel;
}

int Matrix::busAddr() const
{
    return mBusAddr;
}

void Matrix::setBusAddr(int newFrameId)
{
    mBusAddr = newFrameId;
    auto bus = BusManager::inst()->bus(mBusId);
    bus->queryRouter(mBusAddr, mLevel);
}

bool Matrix::setXPoint(int dest, int source)
{
    auto bus = BusManager::inst()->bus(mBusId);
    if (!bus) { //NOTE: this may be a good reason to set an alarm flag
        return false;
    }
    if ((dest<0)||(dest>=numDestinations()))
        return false;
    if ((source<-1)||(source>=numSources()))
        return false;
    mRouting[dest] = source;

    qDebug()<<"set xpoint"<<dest<<source;
    bus->setXPoint(mBusAddr, mLevel, dest, source);
    return true;
}

void Matrix::setLabel(Port::Direction dir, int id, QString label)
{
    //NOTE: this currently is never backpropagated to the matrix

    if (dirIsValid(dir))
        return;
    auto & list = portList(dir);

    if ((id<0)||(id>=list.size()))
        return;

    list[id].label = label;
}

void Matrix::lockPort(int dest)
{
    Q_UNUSED(dest)
    //FIXME: not implemented
}

void Matrix::unlockPort(int dest)
{
    Q_UNUSED(dest)
    //FIXME: not implemented
}

void Matrix::onXPointChanged(QString busId, int addr, int level, int dst, int src)
{
    //TODO: add support for inoput/output offsets
    if (busId != mBusId)
        return;
    if (addr !=mBusAddr)
        return;
    if (level!=mLevel)
        return;
    // if ((dst<mFirstDest) || (dst>=(mFirstDest + mOutputs.size())))

    if ((dst<0) || (dst>=mOutputs.size()))
        return;
    mRouting[dst] = src;
    emit xPointChanged(dst, src);
}

QVector<Matrix::PortInfo> &Matrix::portList(Port::Direction dir)
{
    auto & list = mInputs;
    if (dir == Port::Direction::Destination)
        list = mOutputs;
    return list;
}

QString Matrix::uid() const
{
    return mUid;
}

void Matrix::setUid(const QString &newUid)
{
    mUid = newUid;
}

int Matrix::portFromId(Port::Direction dir, QString id)
{

    QVector<PortInfo> &list = mInputs;
    if (dir == Port::Direction::Destination)
    {
        list = mOutputs;
    }
    else if (dir != Port::Direction::Source)
    {
        return -1;
    }
    int num=0;
    for(auto it = list.begin(); it!=list.end(); ++it)
    {
        if ((it->id == id) || ((!it->uid.isEmpty()) && (it->uid == id)))
            return num;
        ++num;
    }
    return -1;
}

void Matrix::assignIds(bool reset)
{
    int i=1;
    for(auto it = mInputs.begin(); it!=mInputs.end(); ++it)
    {
        if (it->id.isEmpty() || reset)
            it->id = QString::asprintf("in-%02d", i);
        ++i;
    }
    i=1;
    for(auto it = mOutputs.begin(); it!=mOutputs.end(); ++it)
    {
        if (it->id.isEmpty() || reset)
            it->id = QString::asprintf("out-%02d", i);
        ++i;
    }
}

void Matrix::resize(int dests, int srcs)
{
    mInputs.resize(srcs);
    mOutputs.resize(dests);
    mRouting.resize(dests);
    assignIds(false);
}

QString Matrix::busId() const
{
    return mBusId;
}

void Matrix::setBusId(const QString &newBusId)
{
    mBusId = newBusId;
}

bool Matrix::loadConfig(const QJsonObject & cfg)
{
    qDebug()<<cfg;
    // if (cfg.value("uid").toString().isEmpty())
    //     return false;
    if (!checkValue(cfg, "sources", QJsonValue::Array))
        return false;
    if (!checkValue(cfg, "destinations", QJsonValue::Array))
        return false;
    if (!checkValue(cfg, "bus", QJsonValue::String))
        return false;

    mUid      = cfg.value("uid").toString();
    mLevel    = cfg.value("level").toInt(0);
    mBusAddr  = cfg.value("frame_id").toInt();
    mBusId    = cfg.value("bus").toString();
    if (mUid.isEmpty())
        mUid = "rtr-" + mId;

    QJsonArray ins  = cfg.value("sources").toArray();
    QJsonArray outs = cfg.value("destinations").toArray();

    bool ok = true;
    for(auto const & v: std::as_const(ins))
    {
        if (v.isObject()) {
            auto ip = deserialzePortInfo(v.toObject(), &ok);
            mInputs.append(ip);
            if (!ok)
                qWarning()<<"Could not deserialize input port object"<<mInputs.size() + 1<<"on matrix"<<mId;
        } else if (v.isString()) {
            PortInfo port;
            port.uid = mUid + "-src-" + QString::number(mInputs.size() + 1, 10);
            port.label = v.toString();
            mInputs << port;
        } else {
            qWarning()<<"Illegal type for input"<<mInputs.size() + 1<<"on matrix"<<mId;
            PortInfo port;
            port.uid = mUid + "-src-" + QString::number(mInputs.size() + 1, 10);
            port.label = QString::asprintf("input-%d", mInputs.size() + 1);
            mInputs << port;
            continue;
        }
    }
    for(auto const & v: std::as_const(outs))
    {
        if (v.isObject()) {
            auto op = deserialzePortInfo(v.toObject(), &ok);
            mOutputs.append(op);
            if (!ok)
                qWarning()<<"Could not deserialize output port object"<<mOutputs.size() + 1<<"on matrix"<<mId;
        } else if (v.isString()) {
            PortInfo port;
            port.uid = mUid + "-dst-" + QString::number(mOutputs.size() + 1, 10);
            port.label = v.toString();
            mOutputs << port;
        } else {
            qWarning()<<"Illegal type for output"<<mOutputs.size() + 1<<"on matrix"<<mId;
            PortInfo port;
            port.uid = mUid + "-dst-" + QString::number(mOutputs.size() + 1, 10);
            port.label = QString::asprintf("output-%d", mOutputs.size() + 1);
            mOutputs << port;
            continue;
        }
    }
    assignIds(true);
    mRouting.resize(mOutputs.size(), -1);
    return true;
}

QJsonObject Matrix::getConfig() const
{
    QJsonObject cfg;
    cfg.insert("uid", mUid);
    cfg.insert("level", mLevel);
    cfg.insert("frame_id", mBusAddr);
    cfg.insert("bus", mBusId);

    QJsonArray inArray;
    for(auto const & ip: mInputs)
        inArray << serializePortInfo(ip);

    QJsonArray outArray;
    for(auto const & ip: mOutputs)
        outArray << serializePortInfo(ip);

    cfg.insert("inputs",   inArray);
    cfg.insert("outputs", outArray);

    return cfg;
}

} // namespace Router
