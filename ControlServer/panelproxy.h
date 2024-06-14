#ifndef BMD_PANELPROXY_H
#define BMD_PANELPROXY_H



#include <QObject>
#include <QElapsedTimer>

class QTcpSocket;
namespace BMD {

class PanelServer;
class PanelProxy : public QObject
{
    Q_OBJECT
public:
    explicit PanelProxy(QTcpSocket * sock, PanelServer *parent = nullptr);

signals:

private:
    QByteArray mBufferP2H;
    QByteArray mBufferH2P;

    QTcpSocket * mSock;
    QTcpSocket * mRouterConnection;
    void checkFullPackets();
    QElapsedTimer mTimeoutPanel;
    QElapsedTimer mTimeoutServer;

    void debugPrint(QString tag, const QByteArray &d, bool indent=false);
private slots:
    void onDisconnect();
    void onDataReadyPanel();
    void onDataReadyServer();
public slots:

};

} // namespace BMD

#endif // BMD_PANELCONNECTION_H
