#include "routerressource.h"
#include <matrixmanager.h>
#include <http_codes.h>

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>


RouterBaseRessource::RouterBaseRessource(REST::Controller *restController, QObject *parent)
    : QObject{parent}, SplitMethodRessourceInterface{restController}
{}


bool RouterBaseRessource::handleRequest(QString url, REST::HttpContext &context, QByteArray requestBody)
{
    qDebug()<<"handle request for url"<<url<<context.method;
    return REST::delegateRequest(this, url, context, requestBody);
}

void RouterBaseRessource::respondJson(const QJsonDocument & jd, REST::HttpContext & context, int code)
{
    context.response.contentType = "application/json";
    context.response.data = jd.toJson(QJsonDocument::Compact);
    context.response.statusCode = code;
}

void RouterBaseRessource::respondJson(const QJsonObject & obj, REST::HttpContext &context, int code)
{
    respondJson(QJsonDocument(obj), context, code);
}

void RouterBaseRessource::respondJson(const QJsonArray & array, REST::HttpContext &context, int code)
{
    respondJson(QJsonDocument(array), context, code);
}

void RouterBaseRessource::respondJson(const QJsonValue & value, REST::HttpContext &context, int code, const QString &key)
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

QJsonObject RouterBaseRessource::routerStatus(QString id)
{
    auto rman = Router::MatrixManager::inst();
    QJsonObject rtrStatusObj;
    auto rtr = rman->router(id);
    if (!rtr)
    {
        rtrStatusObj.insert("status", "error");
    }
    else
    { //TODO: implement the other parameters
        rtrStatusObj.insert("status", "active");
        rtrStatusObj.insert("alarms", rtr->alarms());
        rtrStatusObj.insert("routing", rtr->getRouting_Json());
        int nsrc = rtr->numSources();
        int ndst = rtr->numDestinations();
        QJsonArray rs;
        rs.append(nsrc);
        rs.append(ndst);
        rtrStatusObj.insert("size", rs);
        QJsonArray ins;
        QJsonArray outs;

        for(int i=0;i<nsrc;++i)
        {
            auto info = rtr->portInfo(Router::Port::Direction::Source, i);
            QJsonObject obj;
            obj.insert("label",    info.label);
            obj.insert("id",       info.id);
            obj.insert("uid",      info.uid);
            obj.insert("mnemonic", info.mnemonic);
            ins.append(obj);
        }
        for(int i=0;i<ndst;++i)
        {
            auto info = rtr->portInfo(Router::Port::Direction::Source, i);
            QJsonObject obj;
            obj.insert("label",    info.label);
            obj.insert("id",       info.id);
            obj.insert("uid",      info.uid);
            obj.insert("mnemonic", info.mnemonic);
            obj.insert("routing",  rtr->getXPoint(i));
            outs.append(obj);
        }
        rtrStatusObj.insert("inputs", ins);
        rtrStatusObj.insert("outputs", outs);
    }
    return rtrStatusObj;
}

bool RouterBaseRessource::handleGet(QString url, REST::HttpContext &context)
{
    auto rman = Router::MatrixManager::inst();
    QStringList path = url.split('/', Qt::SkipEmptyParts);
    QUrlQuery query(context.request.url);
    bool details = false;
    if(query.hasQueryItem("detail"))
        details = true;
    if (path.isEmpty())
    {
        //base dir, return overview
        QStringList uids = rman->getRouterIds();
        QJsonObject status;
        status.insert("uids", QJsonArray::fromStringList(uids));
        if (details)
        {
            QJsonObject statusObject;
            for(auto id: uids)
            {
                statusObject.insert(id, routerStatus(id));
            }
            status.insert("status", statusObject);
        }
        respondJson(status, context);
        context.response.headers.insert("X-Details", details?"true":"false");
        return true;
    }
    if (path.isEmpty())
        return false;
    //path has at least one element
    QString id = path.takeFirst();
    auto router = rman->router(id);
    if(!router)
    {
        context.response.statusCode = 404;
        context.response.contentType = "application/json";
        context.response.data = "{\"error\": \"not found\"}";
        return true;
    }
    else
    {
        if (path.size())
        {
            QString entryId = path.takeFirst();
            QJsonObject obj = routerStatus(id);


            if (path.size())
            {
                int idx = path.takeFirst().toInt();
                if (entryId == "input")
                {
                    if ((idx<0) || (idx>=router->numSources()))
                    {
                        respondJson(QJsonObject(), context, 404);
                        return true;
                    }
                    auto info = router->portInfo(Router::Port::Direction::Source, idx);
                    QJsonObject obj;
                    obj.insert("label",    info.label);
                    obj.insert("id",       info.id);
                    obj.insert("uid",      info.uid);
                    obj.insert("mnemonic", info.mnemonic);
                    respondJson(obj, context, 200);
                    return true;
                }
                else if (entryId=="output")
                {
                    if ((idx<0) || (idx>=router->numDestinations()))
                    {
                        respondJson(QJsonObject(), context, 404);
                        return true;
                    }
                    auto info = router->portInfo(Router::Port::Direction::Source, idx);
                    QJsonObject obj;
                    obj.insert("label",    info.label);
                    obj.insert("id",       info.id);
                    obj.insert("uid",      info.uid);
                    obj.insert("mnemonic", info.mnemonic);
                    obj.insert("routing",  router->getXPoint(idx));
                    respondJson(obj, context, 200);
                    return true;
                }
            }

            if (obj.contains(entryId))
            {
                respondJson(obj.value(entryId), context, 200);
            }
            else
            {
                respondJson(QJsonObject(),context,404);
            }
        }
        else
        {

            if (details)
            {
                respondJson(routerStatus(id), context, 200);
            }
            else
            {
                QJsonObject obj;
                obj.insert("routing", router->getRouting_Json());
                obj.insert("size", QJsonArray()<<router->numSources() <<router->numDestinations());
                obj.insert("alarm", router->alarms());
                obj.insert("status", "active");
                respondJson(obj, context, 200);
            }
        }
    }
    return true;
}

bool RouterBaseRessource::handlePost(QString url, REST::HttpContext &context, QByteArray data)
{
    auto rman = Router::MatrixManager::inst();
    QStringList path = url.split('/', Qt::SkipEmptyParts);
    if (path.size()<2)
    {
        context.response.contentType = "text/plain";
        context.response.data = "400 - Bad request.";
        context.response.statusCode = 400;
        return true;
    }
    if (path.isEmpty())
        return false;

    QString routerId = path.takeFirst();
    auto router = rman->router(routerId);

    if (router) {
        QString action = path.takeFirst();

        //we have a router to control now

        qDebug()<<context.request.query.toString();

        if (action=="set-xpt") {
            qDebug()<< context.request.formData;
            return true;
        }
    }
    context.response.contentType = "text/plain";
    context.response.data = "404 - Not found. (invalid router id)";
    context.response.statusCode = 404;
    return true;

    return false;

}

bool RouterBaseRessource::handlePut(QString url, REST::HttpContext &context, QByteArray data)
{
    return handlePost(url, context, data);
}

bool RouterBaseRessource::handleDelete(QString url, REST::HttpContext &context, QByteArray data)
{
    return false;

}

bool RouterBaseRessource::handlePatch(QString url, REST::HttpContext &context, QByteArray data)
{
    return false;
}
