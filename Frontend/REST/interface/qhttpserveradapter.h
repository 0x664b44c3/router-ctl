#ifndef REST_QHTTPSERVERADAPTER_H
#define REST_QHTTPSERVERADAPTER_H

#include <QObject>
#include <QPointer>
#include "../httpserver/httpresponderinterface.h"


namespace REST {
    class Controller;

class QHttpServerAdapter : public QObject, public HttpResponderInterface
{
    Q_OBJECT
public:
    explicit QHttpServerAdapter(QPointer<Controller> controller, QObject *parent = nullptr);

    // HttpResponderInterface interface
public:
    bool handleRequest(QString url, const QHttpServerRequest &request, QHttpServerResponder &responder) override;

private:
    QPointer<Controller> mController;

signals:

};

} // namespace REST

#endif // REST_QHTTPSERVERADAPTER_H
