#ifndef BUSSERVER_H
#define BUSSERVER_H

#include <QObject>
#include <QWebSocket>
class BusServer : public QObject
{
    Q_OBJECT
public:
    explicit BusServer(QWebSocket * socket, QObject *parent = nullptr);

signals:

protected:
    QWebSocket * mWs;
};

#endif // BUSSERVER_H
