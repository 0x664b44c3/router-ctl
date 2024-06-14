#ifndef HTTPINTERFACE_H
#define HTTPINTERFACE_H

#include <QObject>
#include <QAbstractHttpServer>
#include <QJsonDocument>
class LeitchHack;
class HttpInterface : public QAbstractHttpServer
{
    Q_OBJECT
public:
    explicit HttpInterface(QObject *parent = nullptr);
    ~HttpInterface();

    static QByteArray getPage(QString page, QMap<QString, QString> variables);

signals:

    // QAbstractHttpServer interface
protected:
    bool handleRequest(const QHttpServerRequest &request, QHttpServerResponder &responder);
    void missingHandler(const QHttpServerRequest &request, QHttpServerResponder &&responder);

private:
    QString mIndexPage;
    LeitchHack *mRouter;
};

#endif // HTTPINTERFACE_H
