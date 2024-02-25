#ifndef BUS_IOS_BASE_H
#define BUS_IOS_BASE_H

#include <QObject>

#include <QElapsedTimer>

#include "abstractbusdriver.h"

enum ConnectionType {
    connNone   = 0,
    connSerial = 1,
    connTCP    = 2,
    connTelnet = 3
};


QT_BEGIN_NAMESPACE
class QIODevice;
class QTcpSocket;
class QSerialPort;
QT_END_NAMESPACE

class TelnetClient;

namespace Router {

class IOStreamBusDriverBase : public AbstractBusDriver
{
public:
    IOStreamBusDriverBase(QString busId, QObject * = nullptr);

    bool connectBus() override;
    void disconnectBus() override;
    bool isOnline() const final override;
    // ABstracBusDriver interface is NOT implemented in here
protected:
    QIODevice *mChannel;
    TelnetClient * mTelnet;
    QString mDriverName;
    void setChannel(QIODevice * iodev);
    int mConnectionType;

private:
    //comms functions
    //must be implemented by derived class, gets called by this class
    virtual void processRx(const QByteArray &)=0;

private slots:
    void onTimer();
    void onReadyRead();

protected slots:
    //must be implemented by derived class
    virtual void onChannelConnected() = 0;
    virtual void onChannelDisconnected() = 0;

private:
};

}
#endif // LEITCHBUSDRIVER_H
