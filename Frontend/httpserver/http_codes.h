#ifndef HTTP_CODES_H
#define HTTP_CODES_H
#include <QString>
namespace Http
{

enum StatusCode {

    // Informational responses (100–199)
    statusContinue = 100,
    statusSwitchingProtocols = 101,
    statusProcessing = 102,
    statusEarlyHints = 103,

    // Successful responses (200–299)
    status200_Ok = 200,
    statusCreated = 201,
    statusAccepted = 202,
    statusNonAuthoritativeInfo = 203,
    statusNoContent = 204,
    statusResetContent = 205,
    statusPartialContent = 206,


    // Redirection messages (300–399)
    statusMultipleChoices = 300,
    status301_MovedPermanently = 301,
    statusFound = 302,
    statusSeeOther = 303,
    status307_TemporaryRedirect = 307,


    // Client error responses (400–499)
    statusBadRequest = 400,
    status401_Unauthorized = 401,
    statusForbidden = 403,
    statusNotFound = 404,


    // Server error responses (500–599)
    statusInternalServerError = 500,
    status501_NotImplemented = 501,
    statusBadGateway = 502,
    statusServiceUnavailable = 503

};

    QString statusString(int code);
}
#endif // HTTP_CODES_H
