#ifndef ROUTERRESSOURCE_H
#define ROUTERRESSOURCE_H

#include <QObject>
#include "restressourceinterface.h"

class RouterBaseRessource : public QObject, public REST::SplitMethodRessourceInterface
{
    Q_OBJECT
public:
    explicit RouterBaseRessource(REST::Controller * restController, QObject *parent = nullptr);

signals:

    // RessourceInterface interface
public:
    bool handleRequest(QString url, REST::HttpContext &context, QByteArray requestBody);



    static void respondJson(const QJsonDocument &, REST::HttpContext &context, int code = 200);
    static void respondJson(const QJsonObject   &, REST::HttpContext &context, int code = 200);
    static void respondJson(const QJsonArray    &, REST::HttpContext &context, int code = 200);

    // SplitMethodRessourceInterface interface
public:
    bool handleGet(QString url, REST::HttpContext &context);
    bool handlePut(QString url, REST::HttpContext &context, QByteArray data) override;
    bool handlePost(QString url, REST::HttpContext &context, QByteArray data) override;
    bool handleDelete(QString url, REST::HttpContext &context, QByteArray data) override;
    bool handlePatch(QString url, REST::HttpContext &context, QByteArray data) override;

};

#endif // ROUTERRESSOURCE_H
