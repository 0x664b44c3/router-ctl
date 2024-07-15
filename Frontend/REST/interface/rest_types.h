#ifndef REST_TYPES_H
#define REST_TYPES_H

#include <QString>
#include <QMap>
#include <QByteArray>
#include <QUrl>
#include <QUrlQuery>


namespace REST {

enum Methods : int
{
    UnknownMethod        = 0,
    methodGet            = 0x0001,
    methodPut            = 0x0002,
    methodDelete         = 0x0004,
    methodPost           = 0x0008,
    methodHead           = 0x0010,
    methodOptions        = 0x0020,
    methodPatch          = 0x0040,
    methodConnect        = 0x0080,
    methodTrace          = 0x0100,
    AnyKnownMethod = 0x01ff,
    AnyMethod      = 0x7fffu
};

typedef QFlags<Methods> Method;

struct HttpContext {
    Method method;
    QString methodString;
    struct {
        QUrl url;
        QUrlQuery query;
        QMap<QString, QByteArray> headers;
        QMap<QString, QByteArray> formData;
        QString contentType;
    } request;
    struct {
        QMap<QString, QByteArray> headers;
        QString contentType;
        QByteArray data;
        int statusCode;
    } response;
};

} //namespace
#endif // REST_TYPES_H
