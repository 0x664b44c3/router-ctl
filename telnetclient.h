#ifndef TELNETCLIENT_H
#define TELNETCLIENT_H

#include <QObject>
#include <QHostAddress>
#include <QTcpSocket>

class TelnetClient : public QObject
{
    Q_OBJECT
public:
    explicit TelnetClient(QObject *parent = nullptr);
    ~TelnetClient();
    void connectToHost(QHostAddress addr, quint16 port = 23);
    void disconnect();

    QByteArray getData();
    bool atEnd() const;
    qint64 sendData(QByteArray data);
    bool decodeTelnet() const;
    void setDecodeTelnet(bool newDecodeTelnet);
    QTcpSocket *ioDev() const;
    void flush();
    bool autoReconnect() const;
    void setAutoReconnect(bool newAutoReconnect);

signals:
    void readyRead();
    void disconnected();

//Qt properties
    void decodeTelnetChanged();
signals:
    void connected();

    void autoReconnectChanged();

private slots:
    void onReadyRead();
    void onConnect();
    void onDisconnect();

private:
    QTcpSocket *mSocket;
    bool mDecodeTelnet;
    bool mAutoReconnect;
    Q_PROPERTY(bool decodeTelnet READ decodeTelnet WRITE setDecodeTelnet NOTIFY decodeTelnetChanged FINAL)
    QByteArray mProcBuffer;
    QByteArray mBuffer;
    quint16 mPort;
    QHostAddress mHost;

    void processRx();
    Q_PROPERTY(bool autoReconnect READ autoReconnect WRITE setAutoReconnect NOTIFY autoReconnectChanged FINAL)
};

#endif // TELNETCLIENT_H
