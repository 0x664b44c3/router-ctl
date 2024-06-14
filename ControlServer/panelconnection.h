#ifndef BMD_PANELCONNECTION_H
#define BMD_PANELCONNECTION_H

#include <QObject>

class QTcpSocket;
namespace BMD {

class PanelServer;
class PanelConnection : public QObject
{
    Q_OBJECT
public:
    explicit PanelConnection(QTcpSocket * sock, PanelServer *parent = nullptr);

signals:

private:
    QTcpSocket * mSock;
    void queueOutput(QStringList lines);
    void trySend();
    int mNumInputs, mNumOutputs;
    int mCurrentRun;
    QByteArray mInputBuffer;
    QByteArray mOutputBuffer;
    // status blocks
    QStringList _deviceInfo() const;
    QStringList _statusDump() const;
    void processData();
    void processCmd(QString cmd, QStringList args);
private slots:
    void onDisconnect();
    void onDataReady();
public slots:
    void sendBanner();
};

} // namespace BMD

#endif // BMD_PANELCONNECTION_H
