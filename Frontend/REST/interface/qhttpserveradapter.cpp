#include "qhttpserveradapter.h"
#include "restcontroller.h"
#include "rest_types.h"

#include "../httpserver/httpserver.h"
#include "../httpserver/http_codes.h"

#include <utility>


#if __cplusplus >= 202302L
#define to_baseType(a) std::to_underlying(a)
#else
#define to_baseType(a) static_cast<int>(a)
#endif

namespace REST {

QHttpServerAdapter::QHttpServerAdapter(QPointer<Controller> controller, QObject *parent)
    : QObject{parent}, mController(controller)
{

}

} // namespace REST


bool REST::QHttpServerAdapter::handleRequest(QString url, const QHttpServerRequest &request, QHttpServerResponder &responder)
{

    HttpContext context;
    Method method = UnknownMethod;
    QString methodString;

    switch(request.method())
    {
    case QHttpServerRequest::Method::Get:
        methodString = "GET";
        method = methodGet;
        break;
    case QHttpServerRequest::Method::Put:
        methodString = "PUT";
        method=methodPut;
        break;
    case QHttpServerRequest::Method::Delete:
        methodString = "DELETE";
        method=methodDelete;
        break;
    case QHttpServerRequest::Method::Post:
        methodString = "POST";
        method=methodPost;
        break;
    case QHttpServerRequest::Method::Head:
        methodString = "HEAD";
        method=methodHead;
        break;
    case QHttpServerRequest::Method::Options:
        methodString = "OPTIONS";
        method=methodOptions;
        break;
    case QHttpServerRequest::Method::Patch:
        methodString = "PATCH";
        method=methodPatch;
        break;
    case QHttpServerRequest::Method::Connect:
        methodString = "CONNECT";
        method=methodConnect;
        break;
    case QHttpServerRequest::Method::Trace:
        methodString = "TRACE";
        method=methodTrace;
        break;
    default:
        method = UnknownMethod;
        methodString = "UNKNOWN";
        break;
    }

    context.method = method;
    context.methodString = methodString;
    QByteArray contentType;
    for(auto it = request.headers().begin(); it!= request.headers().end(); ++it)
    {
        QByteArray k = std::get<0>(*it);
        QByteArray v = std::get<1>(*it);

        context.request.headers.insert(QString::fromLatin1(k), v);
        if (k.toLower() == "content-type")
        {
            contentType = v;
        }
    }

    context.request.url = request.url();
    context.request.query = request.query();

    context.request.contentType = contentType;

    context.request.body = request.body();

    if ((request.body().size())
        && ((contentType.startsWith("application/x-www-form-urlencoded")) || (contentType.startsWith("multipart/form-data")))
        ) {
        context.request.formData = HttpServer::parseFormData(contentType, request.body());
    }
    else
    {
    }


    bool ret = mController->handleRequest(url, context, request.body());

    if (!ret) // do not write any response if false is returned
        return false;


    if (context.response.contentType.isEmpty())
        context.response.contentType = "text/plain";

    context.response.headers["Content-Type"] = context.response.contentType.toLatin1();


    QHttpServerResponder::StatusCode code = QHttpServerResponder::StatusCode::BadRequest;

    switch(context.response.statusCode)
    {
        //1xx
    case to_baseType(QHttpServerResponder::StatusCode::Continue):
        code = QHttpServerResponder::StatusCode::Continue;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::SwitchingProtocols):
        code = QHttpServerResponder::StatusCode::SwitchingProtocols;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::Processing):
        code = QHttpServerResponder::StatusCode::Processing;
        break;

    //2xx
    case to_baseType(QHttpServerResponder::StatusCode::Ok):
        code = QHttpServerResponder::StatusCode::Ok;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::Created):
        code = QHttpServerResponder::StatusCode::Created;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::Accepted):
        code = QHttpServerResponder::StatusCode::Accepted;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::NonAuthoritativeInformation):
        code = QHttpServerResponder::StatusCode::NonAuthoritativeInformation;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::NoContent):
        code = QHttpServerResponder::StatusCode::NoContent;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::ResetContent):
        code = QHttpServerResponder::StatusCode::ResetContent;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::PartialContent):
        code = QHttpServerResponder::StatusCode::PartialContent;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::MultiStatus):
        code = QHttpServerResponder::StatusCode::MultiStatus;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::AlreadyReported):
        code = QHttpServerResponder::StatusCode::AlreadyReported;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::IMUsed):
        code = QHttpServerResponder::StatusCode::IMUsed;
        break;


    //3xx
    case to_baseType(QHttpServerResponder::StatusCode::MultipleChoices):
        code = QHttpServerResponder::StatusCode::MultipleChoices;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::MovedPermanently):
        code = QHttpServerResponder::StatusCode::MovedPermanently;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::Found):
        code = QHttpServerResponder::StatusCode::Found;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::SeeOther):
        code = QHttpServerResponder::StatusCode::SeeOther;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::NotModified):
        code = QHttpServerResponder::StatusCode::NotModified;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::UseProxy):
        code = QHttpServerResponder::StatusCode::UseProxy;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::TemporaryRedirect):
        code = QHttpServerResponder::StatusCode::TemporaryRedirect;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::PermanentRedirect):
        code = QHttpServerResponder::StatusCode::PermanentRedirect;
        break;

    //4xx

    case to_baseType(QHttpServerResponder::StatusCode::BadRequest):
        code = QHttpServerResponder::StatusCode::BadRequest;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::Unauthorized):
        code = QHttpServerResponder::StatusCode::Unauthorized;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::PaymentRequired):
        code = QHttpServerResponder::StatusCode::PaymentRequired;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::Forbidden):
        code = QHttpServerResponder::StatusCode::Forbidden;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::NotFound):
        code = QHttpServerResponder::StatusCode::NotFound;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::MethodNotAllowed):
        code = QHttpServerResponder::StatusCode::MethodNotAllowed;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::NotAcceptable):
        code = QHttpServerResponder::StatusCode::NotAcceptable;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::ProxyAuthenticationRequired):
        code = QHttpServerResponder::StatusCode::ProxyAuthenticationRequired;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::RequestTimeout):
        code = QHttpServerResponder::StatusCode::RequestTimeout;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::Conflict):
        code = QHttpServerResponder::StatusCode::Conflict;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::Gone):
        code = QHttpServerResponder::StatusCode::Gone;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::LengthRequired):
        code = QHttpServerResponder::StatusCode::LengthRequired;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::PreconditionFailed):
        code = QHttpServerResponder::StatusCode::PreconditionFailed;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::PayloadTooLarge):
        code = QHttpServerResponder::StatusCode::PayloadTooLarge;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::UriTooLong):
        code = QHttpServerResponder::StatusCode::UriTooLong;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::UnsupportedMediaType):
        code = QHttpServerResponder::StatusCode::UnsupportedMediaType;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::RequestRangeNotSatisfiable):
        code = QHttpServerResponder::StatusCode::RequestRangeNotSatisfiable;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::ExpectationFailed):
        code = QHttpServerResponder::StatusCode::ExpectationFailed;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::ImATeapot):
        code = QHttpServerResponder::StatusCode::ImATeapot;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::MisdirectedRequest):
        code = QHttpServerResponder::StatusCode::MisdirectedRequest;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::UnprocessableEntity):
        code = QHttpServerResponder::StatusCode::UnprocessableEntity;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::Locked):
        code = QHttpServerResponder::StatusCode::Locked;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::FailedDependency):
        code = QHttpServerResponder::StatusCode::FailedDependency;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::UpgradeRequired):
        code = QHttpServerResponder::StatusCode::UpgradeRequired;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::PreconditionRequired):
        code = QHttpServerResponder::StatusCode::PreconditionRequired;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::TooManyRequests):
        code = QHttpServerResponder::StatusCode::TooManyRequests;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::RequestHeaderFieldsTooLarge):
        code = QHttpServerResponder::StatusCode::RequestHeaderFieldsTooLarge;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::UnavailableForLegalReasons):
        code = QHttpServerResponder::StatusCode::UnavailableForLegalReasons;
        break;

    //5xx
    case to_baseType(QHttpServerResponder::StatusCode::InternalServerError):
        code = QHttpServerResponder::StatusCode::InternalServerError;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::NotImplemented):
        code = QHttpServerResponder::StatusCode::NotImplemented;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::BadGateway):
        code = QHttpServerResponder::StatusCode::BadGateway;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::ServiceUnavailable):
        code = QHttpServerResponder::StatusCode::ServiceUnavailable;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::GatewayTimeout):
        code = QHttpServerResponder::StatusCode::GatewayTimeout;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::HttpVersionNotSupported):
        code = QHttpServerResponder::StatusCode::HttpVersionNotSupported;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::VariantAlsoNegotiates):
        code = QHttpServerResponder::StatusCode::VariantAlsoNegotiates;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::InsufficientStorage):
        code = QHttpServerResponder::StatusCode::InsufficientStorage;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::LoopDetected):
        code = QHttpServerResponder::StatusCode::LoopDetected;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::NotExtended):
        code = QHttpServerResponder::StatusCode::NotExtended;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::NetworkAuthenticationRequired):
        code = QHttpServerResponder::StatusCode::NetworkAuthenticationRequired;
        break;

    case to_baseType(QHttpServerResponder::StatusCode::NetworkConnectTimeoutError):
        code = QHttpServerResponder::StatusCode::NetworkConnectTimeoutError;
        break;

    };


    context.response.headers.insert("Content-Length",
                                    QString::number(context.response.data.size(), 10).toLatin1());

    responder.writeStatusLine(code);

    for(auto it = context.response.headers.begin(); it!=context.response.headers.end(); ++it)
    {
        qDebug()<<it.key()<<*it;
        responder.writeHeader(it.key().toLatin1(), *it);

    }

    responder.writeBody(context.response.data);
    return ret;
}
