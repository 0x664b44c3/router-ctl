#include "restcontroller.h"
#include <QUrl>
#include "../httpserver/http_codes.h"

#include "restressourceinterface.h"
namespace REST {

Controller::Controller(QObject *parent)
    : QObject{parent}
{}

static bool match_uri(QString exp, QString uri)
{
    if (exp.endsWith('/'))
    { // match subdirs of path when exp is a path (ending with /)
        if ((uri.startsWith(exp)) || (uri + "/" == exp))
            return true;
    }
    else
    {
        if (uri == exp)
            return true;
    }
    return false;
}

bool Controller::handleRequest(QString url, HttpContext &context, QByteArray requestData)
{
    // qDebug()<<"REST"<<url<<context.methodString;
    for(RessourceInfo rh: std::as_const(mRessources))
    {
        if (match_uri(rh.url, url))
        {
            // qDebug()<<"matched on"<<rh.url;
            if ((rh.methodMask == AnyMethod) || (context.method & rh.methodMask))
            {
                if (!rh.rsrc)
                {
                    genericHttpError(500,context);
                    return true;
                }
                // qDebug()<<"delegate ressource";

                if (rh.url.endsWith('/'))
                    url.remove(0, rh.url.length() - 1); //preserve the /
                else
                    url.remove(0, rh.url.lastIndexOf('/') - 1); //preserve the /

                bool ok = rh.rsrc->handleRequest(url, context, requestData);
                if (!ok)
                {
                    genericHttpError(501, context);
                }
                return true;
            }
            else
            {
                // qDebug()<<"invalid method"<<context.method;
                context.response.data = "bad request.";
                context.response.statusCode = Http::statusBadRequest;
                return true;
            }
        }
    }
    notFound(context);
    return true;
}

void Controller::registerRessource(QString url, RessourceInterface * rsrc, Method methods)
{
    RessourceInfo ri;
    ri.url = url;
    ri.methodMask = methods;
    ri.rsrc = rsrc;
    //put path handlers to the back, single uri handlers to the beginning
    if (url.endsWith('/'))
        mRessources.append(ri);
    else
        mRessources.prepend(ri);
}


void Controller::unregisterRessource(RessourceInterface * ptr)
{
    for(auto it = mRessources.begin(); it!=mRessources.end();)
    {
        if (it->rsrc == ptr)
            it = mRessources.erase(it);
        else
            ++it;
    }
}

void Controller::badRequest(HttpContext &ctx)
{
    ctx.response.contentType = "text/plain";
    ctx.response.statusCode = 400;
    ctx.response.data = "Bad request.";
}

void Controller::notFound(HttpContext &ctx)
{
    ctx.response.contentType = "text/plain";
    ctx.response.statusCode = 404;
    ctx.response.data = "Not found.";
}

void Controller::genericHttpError(int code, HttpContext &ctx)
{
    ctx.response.contentType = "text/plain";
    ctx.response.statusCode = code;
    QString rsp;
    rsp = QString::number(code, 10) + Http::statusString(code);

    ctx.response.data = rsp.toUtf8();
}

QStringList Controller::urlMatches() const
{
    QStringList matches;
    for(RessourceInfo rh: std::as_const(mRessources))
    {
        matches<<rh.url;
    }
    return matches;
}

} // namespace REST
