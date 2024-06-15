#ifndef REST_ROUTER_H
#define REST_ROUTER_H

#include <QObject>
#include <QRegularExpression>
#include "rest_types.h"

namespace REST {
class RessourceInterface;

/**
 * @brief The REST Router class
 * this class implements the lowest layer of a REST server
 * it takes a number of REST::RessourceInterface based
 * ressources that can be registered on-the-fly using a base URI
 * and routes requests from a a server class towards these ressources. *
 *
 */
class Controller : public QObject
{
    Q_OBJECT

public:

    struct RessourceInfo {
        QString url;
        int methodMask;
        RessourceInterface * rsrc;
    };

    explicit Controller(QObject *parent = nullptr);
    /**
     * @brief handleRequest - handle a http(ish) request
     * @param url - the url to be handled
     * @param context - an HttpContext holding request and response data
     * @param requestData - any post/put/patch request body data
     * @return true if the request was handled / the URI matched, no matter of the response code
     */
    bool handleRequest(QString url, HttpContext & context, QByteArray requestData);

    /**
     * @brief registerRessource - register a ressource handler
     * @param url - (base) url of this ressource (if url ends with a '/' all paths starting with this URI will be forwarded to this as well)
     * @param methods - a mask of methods to bind to
     * no ownership of the ressource is taken
     */
    void registerRessource(QString url, RessourceInterface *, Method methods = AnyKnownMethod);
    void unregisterRessource(RessourceInterface *);


    /// \brief badRequest - set the context to contain a 400 "bad request" response
    /// \param ctx - http context
    static void badRequest(HttpContext & ctx);
    /**
     * @brief notFound - setup the context to contain a 404 "not found" response
     * @param ctx - the http context
     */
    static void notFound(HttpContext & ctx);

    static void genericHttpError(int code, HttpContext & ctx);

    QStringList urlMatches() const;

signals:

private:
    QList<RessourceInfo> mRessources;
};

} // namespace REST

#endif // REST_ROUTER_H
