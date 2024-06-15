#ifndef STATICFILESERVER_H
#define STATICFILESERVER_H

#include <QObject>
#include "httpresponderinterface.h"

class staticFileServer : public QObject, public HttpResponderInterface
{
    Q_OBJECT
public:
    explicit staticFileServer(QString rootPath, QObject *parent = nullptr);

private:
    QString mRootPath;

signals:

    // HttpResponderInterface interface
public:
    bool handleRequest(QString url, const QHttpServerRequest &request, QHttpServerResponder &responder);
};

#endif // STATICFILESERVER_H
