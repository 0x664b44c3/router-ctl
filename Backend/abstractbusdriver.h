#ifndef ABSTRACTBUSDRIVER_H
#define ABSTRACTBUSDRIVER_H

#include <QObject>

namespace Router {
class AbstractBusDriver : public QObject
{
    Q_OBJECT
public:
    explicit AbstractBusDriver(QString busId, QObject *parent = nullptr);

    virtual bool rescanBus() {return false;}

    QString deviceUri() const;
    void setDeviceUri(const QString &newDeviceUri);

    virtual bool connectBus() = 0;
    virtual void disconnectBus() = 0;

    bool debug() const;
    void setDebug(bool newDebug);

    const QString &busId() const;

    void setBusId(const QString &newBusId);

    /**
     * @brief isOnline
     * @return true if the bus is connected
     */
    virtual bool isOnline() const = 0;

    virtual QString driverName() const;
    virtual QString driverInfo() const;

    virtual int alarms() const;

public slots:
    virtual void setXPoint(int addr, int level, int dst, int src) = 0;
    virtual void queryRouter(int addr, int level) = 0;
    virtual void queryAlarms(int addr, int level) = 0;

protected:
    QString mDeviceUri;
    QString mBusId;
    bool mDebug;
signals:
    void MatrixFound(
        QString busId,
        int addr, int level, int nSrc, int nDst, int src_offset, int dst_offset);
    void XPointChanged(
        QString busId,
        int addr, int level, int dst, int src);
    void sourceActive(
        QString busId,
        int addr, int level, int src, bool active);
    void connected(QString busId);
    void connectionLost(QString busId);
    void alarmsChanged(QString busId, int addr, int level, int alarms);
};
}

#endif // ABSTRACTBUSDRIVER_H
