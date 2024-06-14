#include "httpinterface.h"
#include <QDebug>
#include <QHttpServerRequest>
#include <QHttpServerResponder>
#include <QHttpServerRouter>
#include <QFile>



static QMap<QString, QString> parseHeaderFields(const QByteArray hdr)
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

static QMap<QString, QByteArray> parseFormData(QByteArray contentType, QByteArray data)
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
                    QString disposition = QString::fromLocal8Bit( line.mid(20, idx-20).trimmed());
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

HttpInterface::HttpInterface(QObject *parent)
    : QAbstractHttpServer{parent}
{
    qDebug() << listen(QHostAddress::Any, 8080);
    mIndexPage = "/web/web_panel.html";
}

HttpInterface::~HttpInterface()
{

}

bool HttpInterface::handleRequest(const QHttpServerRequest &request, QHttpServerResponder &responder)
{

    QUrl url = request.url();
    QString urlPath = url.path().toLower();
    urlPath = urlPath.replace("..","");
    qDebug()<<urlPath;
    return false;

}

void HttpInterface::missingHandler(const QHttpServerRequest &request, QHttpServerResponder &&responder)
{
    QHttpServerResponder::StatusCode code = QHttpServerResponder::StatusCode::NotFound;
    QMap<QString, QString> vars;
    QString path = request.url().path().toHtmlEscaped();
    vars["code"] = QString::asprintf("%03d", (int)code);
    vars["error"] = "Not found.";
    vars["title"] = vars["code"] + " - " + vars["error"];
    vars["message"] = "The URL &quot;" + path + "&quot; was not found on this server.";
    auto data = getPage("error_page.html", vars);
    responder.write(data,"text/html", code);
}


QByteArray HttpInterface::getPage(QString page, QMap<QString, QString> variables)
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
