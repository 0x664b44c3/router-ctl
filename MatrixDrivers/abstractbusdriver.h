#ifndef ABSTRACTBUSDRIVER_H
#define ABSTRACTBUSDRIVER_H

#include <QObject>

namespace Router {
class AbstractBusDriver : public QObject
{
    Q_OBJECT
public:
    explicit AbstractBusDriver(QObject *parent = nullptr);

    virtual bool rescanBus() {return false;}

    QString deviceUri() const;
    void setDeviceUri(const QString &newDeviceUri);

    virtual bool connectBus() = 0;
    virtual void disconnectBus() = 0;


    bool debug() const;
    void setDebug(bool newDebug);

public slots:
    virtual void setXPoint(int addr, int level, int dst, int src) = 0;
    virtual void queryRouter(int addr, int level) = 0;

protected:
    QString mDeviceUri;
    bool mDebug;
signals:
    void MatrixFound(int addr, int level, int nSrc, int nDst, int src_offset, int dst_offset);
    void XPointChanged(int addr, int level, int dst, int src);
    void sourceActive(int addr, int level, int src, bool active);
    void connected();
    void connectionLost();
};
}

#endif // ABSTRACTBUSDRIVER_H
