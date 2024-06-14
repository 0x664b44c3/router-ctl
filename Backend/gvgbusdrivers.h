#ifndef LEITCHBUSDRIVER_H
#define LEITCHBUSDRIVER_H

#include <QObject>

#include <QElapsedTimer>

#include "abstractbusdriver.h"

QT_BEGIN_NAMESPACE
class QIODevice;
class QElapsedTimer;
class QTcpSocket;
class QSerialPort;
QT_END_NAMESPACE

class TelnetClient;

#include "iostreambusbase.h"

namespace Router {

class TenXlAsciiBusDriver : public IOStreamBusDriverBase
{
public:
    TenXlAsciiBusDriver(QString busId, QObject * = nullptr);

    // AbstractBusDriver interface
public:
    bool rescanBus() override;

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

    bool checkBufferForPromt() const;


    QElapsedTimer mTimeSinceCmd;
    QByteArray mBuffer;

    LogonStage mLogonStage;
private slots:
    void onTimer();
    void onChannelConnected();
    void onChannelDisconnected();

private:
    TelnetClient * mTelnet;
};


}
#endif // LEITCHBUSDRIVER_H
