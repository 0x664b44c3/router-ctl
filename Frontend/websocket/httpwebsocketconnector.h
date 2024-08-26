#ifndef HTTPWEBSOCKETCONNECTOR_H
#define HTTPWEBSOCKETCONNECTOR_H

#include <QObject>
#include <QAbstractHttpServer>
namespace REST {
class WebsocketQHttpServerAdapter : public QObject
{
    Q_OBJECT
public:
    explicit WebsocketQHttpServerAdapter(QAbstractHttpServer *httpsrv= nullptr);

signals:
private:
    QAbstractHttpServer *mServer;
private slots:
    void onNewWebSocketConnection();
};
}
#endif // HTTPWEBSOCKETCONNECTOR_H
