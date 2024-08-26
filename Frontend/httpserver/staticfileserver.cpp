#include "staticfileserver.h"
#include "httpserver.h"
#include "mimedb.h"
#include <QFile>
staticFileServer::staticFileServer(QString rootPath, QObject *parent)
    : QObject{parent},mRootPath(rootPath)
{
    if (!mRootPath.endsWith('/'))
        mRootPath.append("/");
}

bool staticFileServer::handleRequest(QString url, const QHttpServerRequest &request, QHttpServerResponder &responder)
{

    if (url.startsWith('/'))
        url.remove(0,1);

    if (url.isEmpty() || url.endsWith("/"))
    {
        url+="index.html";
    }

    url = mRootPath + url;
    // qDebug()<<"Static file server with url"<<url;

    if (request.method() != QHttpServerRequest::Method::Get)
    {
        qDebug()<<"illegal method"<<request.method();
        return false;
    }

    QFile file(url);
    if (!file.exists())
    {
        qDebug()<<file.fileName() << "does not exist";
        HttpServer::notFound(request, responder);
        return true;
    }

    if(!file.open(QFile::ReadOnly))
    {
        QHttpServerResponder::StatusCode code = QHttpServerResponder::StatusCode::Forbidden;
        QMap<QString, QString> vars;
        QString path = request.url().path().toHtmlEscaped();
        vars["code"] = QString::asprintf("%03d", (int)code);
        vars["error"] = "Forbidden.";
        vars["title"] = vars["code"] + " - Forbidden.";
        vars["message"] = "The URL &quot;" + path + "&quot; could not be accessed.\n" + file.errorString();;
        auto data = HttpServer::getPageTemplate("error_page.html", vars);
        responder.write(data, "text/html", code);
        return true;
    }

    //determine MIME type
    QString fileName = url;
    if (fileName.contains('/'))
        fileName = fileName.mid(fileName.lastIndexOf('/') + 1);

    QString extension;
    if (fileName.contains('.'))
        extension = fileName.mid(fileName.lastIndexOf('.') + 1);
    QString contentType = MimeDB::inst()->mime(fileName);
    responder.write(&file, contentType.toLatin1(), QHttpServerResponder::StatusCode::Ok);
    return true;
}
