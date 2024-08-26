#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QAbstractHttpServer>
#include <QJsonDocument>
class LeitchHack;
class HttpResponderInterface;

class HttpServer : public QAbstractHttpServer
{
    Q_OBJECT
public:
    explicit HttpServer(QObject *parent = nullptr);
    ~HttpServer();

    //generic helper to get a page template from internal ressource storage and apply variables to it
    static QByteArray getPageTemplate(QString page, QMap<QString, QString> variables);
    static QMap<QString, QByteArray> parseFormData(QByteArray contentType, QByteArray data);
    static QMap<QString, QString> parseHeaderFields(const QByteArray hdr);

    void missingHandler(const QHttpServerRequest &request, QHttpServerResponder &&responder);
    static void badRequest(const QHttpServerRequest &request, QHttpServerResponder &responder);
    static void notFound(const QHttpServerRequest &request, QHttpServerResponder &responder);

    void registerHandler(QString url, HttpResponderInterface * responder, bool keepPath = false);
signals:

    // QAbstractHttpServer interface
protected:
    bool handleRequest(const QHttpServerRequest &request, QHttpServerResponder &responder);

    struct Handler {
        QString url;
        HttpResponderInterface * responder;
        bool keepPath;
    };
    QList<Handler> mDirHandlers;
    QList<Handler> mUriHandlers;
private:
    QString mIndexPage;
    LeitchHack *mRouter;
private slots:

};

#endif // HTTPSERVER_H
