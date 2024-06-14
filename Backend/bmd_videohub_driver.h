#ifndef BMD_VIDEOHUB_DRIVER_H
#define BMD_VIDEOHUB_DRIVER_H

#include <QObject>

#include <QElapsedTimer>

#include "abstractbusdriver.h"

QT_BEGIN_NAMESPACE
class QIODevice;
class QElapsedTimer;
class QTcpSocket;
class QSerialPort;
QT_END_NAMESPACE

#include "iostreambusbase.h"

namespace Router {

class BmdBusDriver : public IOStreamBusDriverBase
{
public:
    BmdBusDriver(QString busId, QObject * = nullptr);

    // AbstractBusDriver interface
public:
    bool rescanBus() override;
    QString driverName() const override;
    QString driverInfo() const override;

public slots:
    void setXPoint(int addr, int level, int dst, int src) override;
    void queryRouter(int addr, int level) override;
    void queryAlarms(int addr, int level) override;
    //-------

private:
    //comms functions
    void setChannel(QIODevice * iodev);
    void processRx(const QByteArray &);
    void parseLine(const QByteArray & line);
    void checkQueue();

    enum class LogonStage {
        Connected,
        UserSent,
        PassSent,
        LoggedIn
    };

    QStringList mCommandQueue;

    /// true if a prompt character was detected
    bool mHasPrompt;

    bool checkBufferForAck() const;


    QElapsedTimer mTimeSinceCmd;
    QByteArray mBuffer;

    LogonStage mLogonStage;

    int mCurrentRun;
private slots:
    void onTimer();
    void onChannelConnected();
    void onChannelDisconnected();

private:

};

}
#endif // BMD_VIDEOHUB_DRIVER_H
