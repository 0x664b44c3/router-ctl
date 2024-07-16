#include "restressourceinterface.h"
#include "restcontroller.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
bool REST::RessourceInterface::handleRequest(QString url, HttpContext &context, QByteArray requestBody)
{
    Q_UNUSED(url)
    Q_UNUSED(requestBody)
    context.response.contentType = "text/plain";
    context.response.data = QString("Ok.\nYou tried %1 on URI '%1'")
                                .arg(context.methodString)
                                .arg(context.request.url.toString())
                                .toUtf8();
    return true;
}

REST::RessourceInterface::RessourceInterface(Controller *controller) : mController(controller)
{

}

REST::RessourceInterface::~RessourceInterface()
{
    mController->unregisterRessource(this);
}



void REST::RessourceInterface::respondJson(const QJsonDocument & jd, REST::HttpContext & context, int code)
{
    context.response.contentType = "application/json";
    context.response.data = jd.toJson(QJsonDocument::Compact);
    context.response.statusCode = code;
}

void REST::RessourceInterface::respondJson(const QJsonObject & obj, REST::HttpContext &context, int code)
{
    respondJson(QJsonDocument(obj), context, code);
}

void REST::RessourceInterface::respondJson(const QJsonArray & array, REST::HttpContext &context, int code)
{
    respondJson(QJsonDocument(array), context, code);
}

void REST::RessourceInterface::respondJson(const QJsonValue & value, REST::HttpContext &context, int code, const QString &key)
{
    if (value.isObject())
        respondJson(value.toObject(), context, code);
    else if (value.isArray())
    {
        respondJson(value.toArray(), context, code);
    }
    else
    {
        QJsonObject tmpObj;
        tmpObj.insert(key, value);
        respondJson(tmpObj, context, code);
    }
}

void REST::RessourceInterface::respondText(const QString &text, HttpContext &context, int code)
{
    context.response.data = text.toUtf8();
    context.response.contentType = "text/plain; charset=utf-8";
    context.response.statusCode = code;
}

bool REST::SplitMethodRessourceInterface::handleGet(QString url, HttpContext &context)
{
    Q_UNUSED(url)
    Q_UNUSED(context)
    return false;
}

bool REST::SplitMethodRessourceInterface::handlePut(QString url, HttpContext &context, QByteArray data)
{
    Q_UNUSED(url)
    Q_UNUSED(context)
    Q_UNUSED(data)
    return false;
}

bool REST::SplitMethodRessourceInterface::handlePost(QString url, HttpContext &context, QByteArray data)
{
    Q_UNUSED(url)
    Q_UNUSED(context)
    Q_UNUSED(data)
    return false;
}

bool REST::SplitMethodRessourceInterface::handleDelete(QString url, HttpContext &context, QByteArray data)
{
    Q_UNUSED(url)
    Q_UNUSED(context)
    Q_UNUSED(data)
    return false;
}

bool REST::SplitMethodRessourceInterface::handlePatch(QString url, HttpContext &context, QByteArray data)
{
    Q_UNUSED(url)
    Q_UNUSED(context)
    Q_UNUSED(data)
    return false;
}

namespace REST {
bool delegateRequest(SplitMethodRessourceInterface * i, QString url, HttpContext &context, QByteArray requestBody)
{
    switch (context.method)
    {
    case REST::methodGet:
        return i->handleGet(url, context);
        break;
    case REST::methodPut:
        return i->handlePut(url, context, requestBody);
        break;
    case REST::methodPost:
        return i->handlePost(url, context, requestBody);
        break;
    case REST::methodDelete:
        i->handleDelete(url, context, requestBody);
        break;
    case REST::methodPatch:
        i->handlePatch(url, context, requestBody);
        break;
    default:
        return false;
    }
    return false;
}
}
