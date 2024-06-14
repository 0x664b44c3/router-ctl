#include "matrix.h"
#include <tuple>
#include <QJsonArray>
#include "busmanager.h"
#include <abstractbusdriver.h>


static bool checkValue(const QJsonObject & o, QString key, QJsonValue::Type t)
{
    if (!o.contains(key))
        return false;
    if (o[key].type() != t)
        return false;
    return true;
}

typedef std::tuple<QString, QJsonValue::Type> KeyAndType;

static bool checkValues(const QJsonObject & o, std::list<KeyAndType > kt)
{
    for(auto const & t: kt)
    {
        QString key;
        QJsonValue::Type type;
        std::tie(key, type) = t;
        if (!checkValue(o, key, type))
            return false;
    }
    return true;
}

namespace Router {

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

Matrix::Matrix(QString id, QObject *parent)
    : QObject{parent}, mId(id)
{}

int Matrix::alarms() const
{
    return 0;
}

bool Matrix::setBusAddress(QString bus, int level, int frame)
{
    mBusId   = bus;
    mLevel   = level;
    mFrameId = frame;
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

int Matrix::frameId() const
{
    return mFrameId;
}

void Matrix::setFrameId(int newFrameId)
{
    mFrameId = newFrameId;
}

bool Matrix::setXPoint(int dest, int source)
{
    auto bus = BusManager::inst()->bus(mBusId);
    if (!bus) { //NOTE: this may be a good reason to set an alarm flag
        return false;
    }
    bus->setXPoint(mFrameId, mLevel, dest, source);

    return true;
}

void Matrix::setLabel(Port::Direction dir, int id, QString label)
{
    //NOTE: this currently is never backpropagated to the matrix

    if (Router::dirIsValid(dir))
        return;
    auto & list = portList(dir);

    if ((id<0)||(id>=list.size()))
        return;

    list[id].label = label;
}

void Matrix::lockPort(int dest)
{
    //FIXME: not implemented
}

void Matrix::unlockPort(int dest)
{
    //FIXME: not implemented
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
    mFrameId  = cfg.value("frame_id").toInt();
    mBusId    = cfg.value("bus").toString();

    QJsonArray ins  = cfg.value("sources").toArray();
    QJsonArray outs = cfg.value("destinations").toArray();

    bool ok = true;
    for(auto const & v: qAsConst(ins))
    {
        if (v.isObject()) {
            auto ip = deserialzePortInfo(v.toObject(), &ok);
            mInputs.append(ip);
            if (!ok)
                qWarning()<<"Could not deserialize input port object"<<mInputs.size() + 1<<"on matrix"<<mId;
        } else if (v.isString()) {
            PortInfo port;
            port.label = v.toString();
            mInputs << port;
        } else {
            qWarning()<<"Illegal type for input"<<mInputs.size() + 1<<"on matrix"<<mId;
            PortInfo port;
            port.label = QString::asprintf("input-%d", mInputs.size() + 1);
            mInputs << port;
            continue;
        }
    }
    for(auto const & v: qAsConst(outs))
    {
        if (v.isObject()) {
            auto ip = deserialzePortInfo(v.toObject(), &ok);
            mOutputs.append(ip);
            if (!ok)
                qWarning()<<"Could not deserialize output port object"<<mInputs.size() + 1<<"on matrix"<<mId;
        } else if (v.isString()) {
            PortInfo port;
            port.label = v.toString();
            mOutputs << port;
        } else {
            qWarning()<<"Illegal type for output"<<mInputs.size() + 1<<"on matrix"<<mId;
            PortInfo port;
            port.label = QString::asprintf("output-%d", mInputs.size() + 1);
            mOutputs << port;
            continue;
        }
    }
    assignIds(true);
    return true;
}

QJsonObject Matrix::getConfig() const
{
    QJsonObject cfg;
    cfg.insert("uid", mUid);
    cfg.insert("level", mLevel);
    cfg.insert("frame_id", mFrameId);
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
