#ifndef ROUTER_BUSMANAGER_H
#define ROUTER_BUSMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVariantMap>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
class QJsonObject;
QT_END_NAMESPACE
#include <QElapsedTimer>

template <class T> class ObjectFactory;

namespace Router {
class AbstractBusDriver;

class BusManager : public QObject
{
    Q_OBJECT
public:
    static BusManager *inst();

    enum {
        alarmBusOffline      = 0x0001,
        alarmConnectFailed   = 0x0010,
        alarmNoSuchDriver    = 0x0020,

        alarmHwRefLost       = 0x0400,

        alarmHwPsuFailed     = 0x1000,
        alarmSwInternalError = 0x2000,
        alarmHwGenericError  = 0x4000,
        alarmHwFanError      = 0x8000

    };

    bool load(const QJsonObject &);
    int alarms() const;
    int alarmsForBus(QString busId) const;
    void debugBusState() const;
    QStringList getBusIds() const;
    
    AbstractBusDriver * bus(QString id) const;
    
    QStringList alarmStrings(QString busId) const;
private:
    explicit BusManager(QObject *parent = nullptr);

    void checkBusses();
    struct BusEntry
    {
        QElapsedTimer     offlineTimer;
        QString driver;
        QString url;
        bool wasStarted;
        QVariantMap parameters;
        AbstractBusDriver *bus;
        int alarm;
    };
    static BusManager * mInst;
    ObjectFactory<AbstractBusDriver> *mDriverFactory;
    QMap<QString, BusEntry> mBusses;
    int mAlarmState;

    bool mDebug;
signals:

    void matrixFound(
        QString busId,
        int addr, int level, int nSrc, int nDst, int src_offset, int dst_offset);
    void xPointChanged(
        QString busId,
        int addr, int level, int dst, int src);
    void sourceActive(
        QString busId,
        int addr, int level, int src, bool active);
    void busConnected(QString busId);
    void busConnectionLost(QString busId);
    void busEvent(QString type, const QJsonObject data);
public slots:

private slots:
    void onTimer();
    //slots for bus drivers' signals
    void onMatrixFound(
        QString busId,
        int addr, int level, int nSrc, int nDst, int src_offset, int dst_offset);
    void onXPointChanged(
        QString busId,
        int addr, int level, int dst, int src);
    void onSourceActive(
        QString busId,
        int addr, int level, int src, bool active);
    void onDrvConnected(QString busId);
    void onDrvConnectionLost(QString busId);
};

} // namespace Router

#endif // ROUTER_BUSMANAGER_H
