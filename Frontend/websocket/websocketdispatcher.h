#ifndef WEBSOCKETDISPATCHER_H
#define WEBSOCKETDISPATCHER_H

#include <QObject>

class WebSocketDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketDispatcher(QObject *parent = nullptr);

signals:
};

#endif // WEBSOCKETDISPATCHER_H
