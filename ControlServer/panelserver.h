#ifndef BMD_PANELSERVER_H
#define BMD_PANELSERVER_H

#include <QObject>
#include <panelconnection.h>
class QTcpServer;
class QTcpSocket;
namespace BMD {
class PanelConnection;

class PanelServer : public QObject
{
    Q_OBJECT
public:
    explicit PanelServer(quint16 port = 9990, QObject *parent = nullptr);

signals:

private:
    QTcpServer *mServer;
    qint16 mTcpPort;
    int mInputs;
    int mOutputs;
private slots:
    void onNewConnection();
};

} // namespace BMD


#endif // BMD_PANELSERVER_H
