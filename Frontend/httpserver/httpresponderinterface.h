#ifndef HTTPRESPONDERINTERFACE_H
#define HTTPRESPONDERINTERFACE_H

#include <QHttpServerRequest>
#include <QHttpServerResponder>

class HttpResponderInterface
{
public:
    explicit HttpResponderInterface();
    virtual bool handleRequest(QString url, const QHttpServerRequest &request, QHttpServerResponder &responder) = 0;
};

#endif // HTTPRESPONDERINTERFACE_H
