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
    context.response.data = jd.toJson();
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

bool RouterBaseRessource::handleGet(QString url, REST::HttpContext &context)
{
    QStringList path = url.split('/', Qt::SkipEmptyParts);
    qDebug()<<path;
    if (path.isEmpty())
    {
        bool tree=false;
        //base dir, return overview
        QJsonObject status;
        status.insert("routers", QJsonArray());
        respondJson(status, context);
        context.response.headers.insert("X-Details", tree?"true":"false");
    }
    return true;
}

bool RouterBaseRessource::handlePost(QString url, REST::HttpContext &context, QByteArray data)
{

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
