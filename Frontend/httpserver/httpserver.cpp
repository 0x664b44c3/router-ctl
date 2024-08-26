#include "httpserver.h"
#include <QDebug>
#include <QHttpServerRequest>
#include <QHttpServerResponder>
#include <QHttpServerRouter>
#include <QFile>

#include "mimedb.h"

#include "httpresponderinterface.h"

QMap<QString, QString> HttpServer::parseHeaderFields(const QByteArray hdr)
{
    QMap<QString, QString> data;
    bool inQuote=false;
    QString name;
    QString value;
    int idx=0;
    int start=0;
    while(idx<hdr.length())
    {
        while((idx<hdr.length()) && (isspace(hdr[idx]) || (hdr[idx] == ';')))
            ++idx;
        start=idx;
        idx = hdr.indexOf('=', idx);
        if (idx<0)
        {
            qDebug()<<"no = found";
            break;
        }
        name = QString::fromLatin1(hdr.mid(start, idx - start));
        //scan for ; or end of line
        ++idx;
        start=idx;
        while(idx<hdr.length())
        {
            if (hdr[idx] == '"')
                inQuote = !inQuote;
            if ((inQuote==false) && (hdr[idx] == ';'))
                break;
            ++idx;
        }
        QString value = QString::fromLatin1(hdr.mid(start, idx - start));
        if (value.startsWith('"'))
        {
            value = value.mid(1).chopped(1);
        }
        data.insert(name, value);
    }
    return data;
}

QMap<QString, QByteArray> HttpServer::parseFormData(QByteArray contentType, QByteArray data)
{
    QMap<QString, QByteArray> formData;

    QString ct_lower = QString::fromLatin1(contentType).toLower();
    if (ct_lower == "application/x-www-form-urlencoded")
    {
        QList<QByteArray> formDataFields = data.split('&');
        for(QByteArray entry: formDataFields)
        {
            int vs = entry.indexOf('=');

            QByteArray fieldName, fieldValue;
            if (vs<0)
                vs=entry.size();
            fieldName = QByteArray::fromPercentEncoding(entry.left(vs));
            fieldValue = QByteArray::fromPercentEncoding(entry.mid(vs+1));
            formData.insert(QString::fromLocal8Bit(fieldName), fieldValue);
        }
        return formData;
    }
    if (ct_lower.startsWith("multipart/form-data"))
    {
        QByteArray ct_raw = contentType;

        QByteArray boundary("---");
        int boundaryIdx = ct_raw.indexOf("boundary=");
        if (boundaryIdx)
            boundary = ct_raw.mid(boundaryIdx + 9);

        QList<QByteArray> parts;
        qint64 idx = data.indexOf(boundary);
        qint64 lastBoundaryEnd=-1;

        while(idx>=0)
        {
            //find end of line
            qint64 boundary_start = idx;
            while(boundary_start>0)
            {
                char leftOfBoundary = data.at(boundary_start-1);
                if (leftOfBoundary == '\n')
                    break;
                if (leftOfBoundary == '\r')
                    break;
                --boundary_start;
            }
            idx+=boundary.size();
            while(idx<data.size())
            {
                if ((data.at(idx)=='\n') || (data.at(idx)=='\r'))
                    break;
                ++idx;
            }
            while((data.at(idx)=='\n') || (data.at(idx)=='\r'))
            {
                ++idx;
                if (idx>=data.size())
                    break;
            }

            if (lastBoundaryEnd>=0)
            {
                parts.push_back(data.mid(lastBoundaryEnd, boundary_start-lastBoundaryEnd));
            }
            if (idx>=data.size())
                break;
            lastBoundaryEnd = idx;
            idx = data.indexOf(boundary, lastBoundaryEnd);
        }

        foreach(QByteArray part, parts)
        {
            QString fieldName;
            bool isFieldData=false;
            bool headersDone=false;
            int start=0;
            int next=part.indexOf('\n');
            QByteArray fieldData;
            while(next>=0)
            {
                QByteArray line = part.mid(start, next-start+1);
                start=next+1;
                next = part.indexOf('\n', start);
                if ((!isFieldData) && (line.startsWith("Content-Disposition:")))
                { //20 is length of header name
                    int idx = line.indexOf(';', 20);
                    if (idx<0)
                        continue;
                    QString disposition = QString::fromLatin1(line.mid(20, idx-20).trimmed());
                    auto headers = parseHeaderFields(line.mid(idx).trimmed());
                    if (!headers.contains("name")) // parts with no name are ignored.
                        break;
                    isFieldData = true;
                    fieldName = headers.value("name");
                    continue;
                }
                if (isFieldData && line.trimmed().isEmpty())
                {
                    headersDone = true;
                    continue;
                }
                if ((isFieldData) && (headersDone))
                {
                    fieldData.append(line);
                }
            }
            if (!fieldName.isEmpty())
                formData.insert(fieldName, fieldData);
        }
        return formData;
    }

    formData.clear();
    return formData;
}

HttpServer::HttpServer(QObject *parent)
    : QAbstractHttpServer{parent}
{
    MimeDB::inst();
    qDebug() <<"Web server listen: "<< listen(QHostAddress::Any, 8080);
    mIndexPage = "/web/web_panel.html";
}

HttpServer::~HttpServer()
{

}

static bool match_uri(QString exp, QString uri)
{
    if (exp.endsWith('/'))
    { // match subdirs of path when exp is a path (ending with /)
        if ((uri.startsWith(exp)) || (uri + "/" == exp))
            return true;
    }
    else
    {
        if (uri == exp)
            return true;
    }
    return false;
}

bool HttpServer::handleRequest(const QHttpServerRequest &request, QHttpServerResponder &responder)
{

    QUrl url = request.url();
    QString urlPath = url.path().toLower();
    urlPath = urlPath.replace("..", "");

    QString ressourcePath;

    QMap<QString, QByteArray> headers;
    for(auto it = request.headers().begin(); it!= request.headers().end(); ++it)
    {
        QByteArray k = std::get<0>(*it);
        QByteArray v = std::get<1>(*it);

        headers.insert(QString::fromLatin1(k), v);
    }
    if (headers.value("Upgrade").toLower() == "websocket")
        return true;

    HttpResponderInterface * rsp = nullptr;

    bool keepPath = false;
    for(Handler rh: std::as_const(mUriHandlers))
    {
        if (match_uri(rh.url, urlPath))
        {
            ressourcePath = rh.url;
            rsp = rh.responder;
            break;
        }
    }

    if (!rsp) {
        for(Handler rh: std::as_const(mDirHandlers))
        {
            if (match_uri(rh.url, urlPath))
            {
                ressourcePath = rh.url;
                rsp           = rh.responder;
                keepPath      = rh.keepPath;
                break;
            }
        }
    }

    if (!rsp)
    {
        return false;
    }
    else
    {
        if (!keepPath)
        {
            if (ressourcePath.endsWith('/'))
                urlPath.remove(0, ressourcePath.length() - 1); //preserve the /
            else
                urlPath.remove(0, ressourcePath.lastIndexOf('/') - 1); //preserve the /
        }

        // qDebug()<<"Calling handler for" << urlPath<<"from url"<<request.url().toString();

        bool ret = rsp->handleRequest(urlPath, request, responder);
        if (!ret)
        {
            badRequest(request, responder);
        }

        return true;
    }
    return false;
}


void HttpServer::missingHandler(const QHttpServerRequest &request, QHttpServerResponder &&responder)
{
    QHttpServerResponder::StatusCode code = QHttpServerResponder::StatusCode::NotFound;
    QMap<QString, QString> vars;
    QString path = request.url().path().toHtmlEscaped();
    vars["code"] = QString::asprintf("%03d", (int)code);
    vars["error"] = "Not found.";
    vars["title"] = vars["code"] + " - " + vars["error"];
    vars["message"] = "The URL &quot;" + path + "&quot; was not found on this server.";
    auto data = getPageTemplate("error_page.html", vars);
    responder.write(data,"text/html", code);
}


void HttpServer::badRequest(const QHttpServerRequest &request, QHttpServerResponder &responder)
{
    QHttpServerResponder::StatusCode code = QHttpServerResponder::StatusCode::BadRequest;
    QMap<QString, QString> vars;
    QString path = request.url().path().toHtmlEscaped();
    vars["code"] = QString::asprintf("%03d", (int)code);
    vars["error"] = "Bad request.";
    vars["title"] = vars["code"] + " - " + vars["error"];
    vars["message"] = "The URL &quot;" + path + "&quot; cannot be accessed using this method.";
    auto data = getPageTemplate("error_page.html", vars);
    responder.write(data,"text/html", code);
}

void HttpServer::notFound(const QHttpServerRequest &request, QHttpServerResponder &responder)
{
    QHttpServerResponder::StatusCode code = QHttpServerResponder::StatusCode::NotFound;
    QMap<QString, QString> vars;
    QString path = request.url().path().toHtmlEscaped();
    vars["code"] = QString::asprintf("%03d", (int)code);
    vars["error"] = "Not found.";
    vars["title"] = vars["code"] + " - " + vars["error"];
    vars["message"] = "The URL &quot;" + path + "&quot; was not found on this server.";;
    auto data = getPageTemplate("error_page.html", vars);
    responder.write(data,"text/html", code);
}


void HttpServer::registerHandler(QString url, HttpResponderInterface *responder, bool keepPath)
{
    Handler h;
    h.url = url;
    h.responder = responder;
    h.keepPath = keepPath;
    //put path handlers to the back, single uri handlers to the beginning
    if (url.endsWith('/'))
        mDirHandlers.append(h);
    else
        mUriHandlers.append(h);

    //sort list with stable sort to put shorter matches to the end


    std::stable_sort(mDirHandlers.begin(), mDirHandlers.end(), [](const Handler & a, const Handler & b){return a.url.length()>=b.url.length();});

}


QByteArray HttpServer::getPageTemplate(QString page, QMap<QString, QString> variables)
{
    page = page.replace("..", "");
    QFile file(":/webui/" + page);
    if (!file.open(QFile::ReadOnly))
    {
        return QByteArray();
    }
    QString pageData = QString::fromUtf8(file.readAll());
    for(auto it = variables.begin(); it!=variables.end(); ++it)
    {
        pageData = pageData.replace("%"+it.key()+"%", it.value());
    }
    return pageData.toUtf8();
}
