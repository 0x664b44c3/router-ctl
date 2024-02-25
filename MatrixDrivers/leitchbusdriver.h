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

namespace Router {

class LeitchBusDriver : public AbstractBusDriver
{
public:
    LeitchBusDriver(QObject * = nullptr);

    // AbstractBusDriver interface
public:
    bool rescanBus() override;
    bool connectBus() override;
    void disconnectBus() override;

public slots:
    void setXPoint(int addr, int level, int dst, int src) override;
    void queryRouter(int addr, int level) override;
    //-------

protected:
    QIODevice *mChannel;

public:

protected:

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
    void onReadyRead();
    void onChannelConncted();

private:
    TelnetClient * mTelnet;
    int mConnectionType;
};

}
#endif // LEITCHBUSDRIVER_H
