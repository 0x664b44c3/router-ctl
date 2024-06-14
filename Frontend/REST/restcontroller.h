#ifndef RESTCONTROLLER_H
#define RESTCONTROLLER_H

#include <QObject>

class RestController : public QObject
{
    Q_OBJECT
public:
    explicit RestController(QObject *parent = nullptr);

signals:
};

#endif // RESTCONTROLLER_H
