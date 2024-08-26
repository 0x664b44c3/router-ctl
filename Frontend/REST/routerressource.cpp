#include "routerressource.h"
#include <matrixmanager.h>
#include <http_codes.h>

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>


PanelInterface::PanelInterface(REST::Controller *restController, QObject *parent)
    : QObject{parent}, SplitMethodRessourceInterface{restController}
{}


bool PanelInterface::handleRequest(QString url, REST::HttpContext &context, QByteArray requestBody)
{
    bool ok = REST::delegateRequest(this, url, context, requestBody);
    if (!ok)
        qDebug()<<"unhandled request for url"<<url<<context.method;
    return ok;
}

QJsonObject PanelInterface::JsonForPort(Router::Matrix *mtx, Router::Port::Direction dir, int idx)
{
    auto port = mtx->portInfo(dir, idx);
    QJsonObject obj;
    obj.insert("id",       port.id);
    obj.insert("uid",      port.uid);
    obj.insert("label",    port.label);
    obj.insert("mnemonic", port.mnemonic);

    if (dir == Router::Port::Direction::Destination) {
        int xpoint = mtx->getXPoint(idx);
        obj.insert("xpoint",  xpoint );
        obj.insert("routing",  mtx->portInfo(Router::Port::Direction::Source, xpoint).uid);
    }
    return obj;
}

QJsonObject PanelInterface::routerStatus(QString id)
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
        rtrStatusObj.insert("alarm", rtr->alarms());
        rtrStatusObj.insert("xpoint", rtr->getXPointsJson());
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
            ins.append(JsonForPort(rtr, Router::Port::Direction::Source, i));
        }
        for(int i=0;i<ndst;++i)
        {

            outs.append(
                JsonForPort(rtr, Router::Port::Direction::Destination, i)
                );
        }
        rtrStatusObj.insert("inputs", ins);
        rtrStatusObj.insert("outputs", outs);
    }
    return rtrStatusObj;
}

bool PanelInterface::handleGet(QString url, REST::HttpContext &context)
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
                    QJsonObject obj = JsonForPort(router, Router::Port::Direction::Source, idx);
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
                    QJsonObject obj = JsonForPort(router, Router::Port::Direction::Destination, idx);
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
                obj.insert("xpoint", router->getXPointsJson());
                obj.insert("size", QJsonArray()<<router->numSources() <<router->numDestinations());
                obj.insert("alarm", router->alarms());
                obj.insert("status", "active");
                respondJson(obj, context, 200);
            }
        }
    }
    return true;
}

bool PanelInterface::handlePost(QString url, REST::HttpContext &context, QByteArray data)
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
        int src=-1;
        int dst=-1;
        bool ok;

        if (action=="xpoint-set") {
            if (!context.request.formData.isEmpty())
            {
                src = context.request.formData.value("src", "-1").toInt(&ok);
                if (ok)
                    dst = context.request.formData.value("dst", "-1").toInt(&ok);
            }
            if (!ok)
            {
                respondText("Invalid data.", context, 400);
                return true;
            }
            if (ok && (dst>=0))
                router->setXPoint(dst, src);
            respondText("Ok.", context);
            return true;
        }
    }
    context.response.contentType = "text/plain";
    context.response.data = "404 - Not found. (invalid router id)";
    context.response.statusCode = 404;
    return true;

    return false;

}

bool PanelInterface::handlePut(QString url, REST::HttpContext &context, QByteArray data)
{
    return handlePost(url, context, data);
}

bool PanelInterface::handleDelete(QString url, REST::HttpContext &context, QByteArray data)
{
    return false;

}

bool PanelInterface::handlePatch(QString url, REST::HttpContext &context, QByteArray data)
{
    return false;
}
