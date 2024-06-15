#include "restressourceinterface.h"
#include "restcontroller.h"

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
