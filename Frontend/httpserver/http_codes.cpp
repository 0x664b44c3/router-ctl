#include "http_codes.h"
#include <QString>

namespace Http {

QString statusString(int code)
{
    switch(code)
    {

    case statusContinue:
        return "Continue";
    case statusSwitchingProtocols:
        return "Switching protocols";
    case statusProcessing:
        return "Processing";
    case statusEarlyHints:
        return "Early Status Hints";

        // Successful responses (200–299)
    case status200_Ok:
        return "OK";
    case statusCreated:
        return "Created";
    case statusAccepted:
        return "Accepted";
    case statusNonAuthoritativeInfo:
        return "Non-Authorative Information";
    case statusNoContent:
        return "No Content";
    case statusResetContent:
        return  "Reset Content";
    case statusPartialContent:
        return "Partial Content";

        // Redirection messages (300–399)
    case statusMultipleChoices:
        return "Multiple Choices";
    case status301_MovedPermanently:
        return "Moved Permanently";
    case statusFound:
        return "Found";
    case statusSeeOther:
        return "See Other";


        // Client error responses (400–499)
    case statusBadRequest:
        return "Bad Request";
    case status401_Unauthorized:
        return "Unauthorized";
    case statusForbidden:
        return "Forbidden";
    case statusNotFound:
        return "Not Found";


        // Server error responses (500–599)
    case statusInternalServerError:
        return "Internal Server Error";
    case status501_NotImplemented:
        return "Not Implemented";
    case statusBadGateway:
        return "Bad Gateway";
    case statusServiceUnavailable:
        return "Service Unavailable";
    default:
        break;
    }
    return "Unknown";
};
}
