#ifndef RESTRESSOURCEINTERFACE_H
#define RESTRESSOURCEINTERFACE_H

#include <QtGlobal>
#include <QString>
#include <QMap>
#include <QByteArray>
#include <QUrl>

#include <QJsonDocument>
#include "rest_types.h"
namespace REST {

class Controller;

class RessourceInterface
{
public:
    /**
     * @brief handleRequest - handle a request
     * @param url - the url that was called
     * @param context - a @struct of type REST::HttpContext that holds all data associated to this request, incl. the response
     * @param requestBody - the request body if any
     * @return true if request was handled, false if not. @note you MUST return true if you want any
     * @return of your response to be sent out to the client.
     */
    virtual bool handleRequest (QString url, HttpContext & context, QByteArray requestBody);
    explicit RessourceInterface(Controller * controller);
    ~RessourceInterface();

    //static helpers

    static void respondJson(const QJsonDocument &, REST::HttpContext &context, int code = 200);
    static void respondJson(const QJsonObject   &, REST::HttpContext &context, int code = 200);
    static void respondJson(const QJsonArray    &, REST::HttpContext &context, int code = 200);
    static void respondJson(const QJsonValue    &, REST::HttpContext &context, int code = 200, const QString &key = "value");
    static void respondText(const QString & text, REST::HttpContext &context, int code = 200);


protected:
    Controller * mController;
};

class SplitMethodRessourceInterface : public RessourceInterface {
public:
    virtual bool handleGet    (QString url, HttpContext & context);
    virtual bool handlePut    (QString url, HttpContext & context, QByteArray data);
    virtual bool handlePost   (QString url, HttpContext & context, QByteArray data);
    virtual bool handleDelete (QString url, HttpContext & context, QByteArray data);
    virtual bool handlePatch  (QString url, HttpContext & context, QByteArray data);
    explicit SplitMethodRessourceInterface(Controller * controller) : RessourceInterface{controller} {}
};

bool delegateRequest(SplitMethodRessourceInterface *, QString url, HttpContext & context, QByteArray requestBody);

}
#endif // RESTRESSOURCEINTERFACE_H
