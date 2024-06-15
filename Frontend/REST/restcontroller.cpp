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
    for(RessourceInfo rh: std::as_const(mRessources))
    {
        if (match_uri(rh.url, url))
        {
            if ((rh.methodMask == AnyMethod) || (context.method & rh.methodMask))
            {
                return rh.rsrc->handleRequest(url, context, requestData);
            }
            else
            {
                context.response.data = "bad request.";
                context.response.statusCode = Http::statusBadRequest;
                return true;
            }
        }
    }
    return false;
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

QStringList Controller::urlMatches() const
{
    QStringList matches;
    for(RessourceInfo rh: std::as_const(mRessources))
    {
        matches<<rh.url;
    }
}

} // namespace REST
