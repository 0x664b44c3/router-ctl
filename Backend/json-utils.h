#ifndef JSONUTILS_H
#define JSONUTILS_H
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <tuple>

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

#endif // JSONUTILS_H
