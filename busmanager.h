#ifndef ROUTER_BUSMANAGER_H
#define ROUTER_BUSMANAGER_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QJsonObject;
QT_END_NAMESPACE

namespace Router {

class AbstractBusDriver;

class BusManager : public QObject
{
    Q_OBJECT
public:
    static BusManager *inst();
    bool load(const QJsonObject &);

private:

    explicit BusManager(QObject *parent = nullptr);    
    static BusManager * mInst;
signals:
};

} // namespace Router

#endif // ROUTER_BUSMANAGER_H
