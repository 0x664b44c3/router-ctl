#ifndef ROUTERMANAGER_H
#define ROUTERMANAGER_H

#include <QObject>

class RouterManager : public QObject
{
    Q_OBJECT
public:
    explicit RouterManager(QObject *parent = nullptr);

signals:

};

#endif // ROUTERMANAGER_H
