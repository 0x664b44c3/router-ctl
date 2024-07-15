/** video router control server
 * This tool allows the control of several SDI, audio or control routers as used in broadcast applications.
 * This tool consists of several layers that each take care of a certain aspect of the control operation
 * The user interface is provided as a set of simple web-softpanels as well as several protocols.
 * This program can be expanded with backends for various kinds o fsignal routing equipment
 * as well as frontend servers that allow connection of existing hardware panels to the system,
 * reimplementing various vendor-specific protocols.
 */
/**
 * Architecture
 *
 * BACKEND
 *  - control protocol backends (clients towards existing gear (signal routers)
 *  - router manager -  a central management layer that loads and configures the backend links ("busses")
 *    as well as takes care of monitoring and alarm management.
 *
 * CORE
 *  - physical matrix layer - the MODEL layer
 *    a representation of the actual crosspoint switches, controls and monitors the routers
 *    through the busses of the router manager.
 *  - router core - the CONTROLLER
 *    this is the core component that maps routing requests to the matrix layer
 *    also manages tie lines between routers and handles locking of outputs
 *  - logical matrix (panel) layer, the VIEW
 *    exports the sources and destinations of the router system.
 *    "virtual routers" can be defined from any number of sources and destinations of any router in
 *    the system.
 *
 * FRONTEND
 *  - frontend servers
 *    several control interfaces like RESTful, Websocket, TCP, etc
 *    allows connecting browser based virtual panels, hardware panels or other
 *    control software to the system
 **/
#include <QCoreApplication>

#include <QDebug>
#include <QTimer>

#include "routercore.h"
#include <umdcontroller.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>

#include <QTextStream>

#include <busmanager.h>
#include <abstractbusdriver.h>
#include <matrixmanager.h>

#include <routerressource.h>
#include <httpserver.h>
#include <staticfileserver.h>
#include <restcontroller.h>
#include <qhttpserveradapter.h>

#include <QUrl>
void debugMessageHandler(QtMsgType mType, const QMessageLogContext & ctx, const QString & msg)
{
    Q_UNUSED(ctx)
    QString message;
    // int prio = LOG_DEBUG;
    switch(mType)
    {
    case QtDebugMsg:
        message+="[debug]   ";
        break;

    case QtInfoMsg:
        message+="[info]    ";
        break;
    case QtWarningMsg:
        message+="[warning] ";
        // prio = LOG_WARNING;
        break;
    case QtCriticalMsg:
        message+="[critical]";
        // prio = LOG_CRIT;
        break;
    case QtFatalMsg:
        message+="[fatal]   ";
        // prio = LOG_ALERT;
        break;
    // case QtInfoMsg:
        // prio = LOG_INFO;
    default:
        break;
    }
    message += " ";
    message+=msg;
    message+=QString(" (%1:%2)").arg(ctx.file).arg(ctx.line);
    QTextStream stdError(stderr);
    stdError << message << "\n";
}


void showConfig(QTextStream & stream) {

    stream << "Config info:\n\n";
    stream << "BUSSES:\n";

    auto busMgr = Router::BusManager::inst();
    auto mtxMgr = Router::MatrixManager::inst();

    auto mtxNames = mtxMgr->getRouterIds();
    auto busNames = busMgr->getBusIds();

    for(auto busId: std::as_const(busNames))
    {
        stream << "  ";
        auto bus = busMgr->bus(busId);
        stream << "Bus "<< busId<<" ";
        if (!bus)
        {
            stream << "not loaded.\n";
            continue;
        }
        stream << "(" << bus->driverName() << "): ";
        if (bus->isOnline())
            stream << "ONLINE ";
        else
            stream << "OFFLINE ";
        int alm = busMgr->alarmsForBus(busId);
        if (alm)
            stream << "ALM: 0x"<<QString::number(alm,16);
        else
            stream << "OK";
        stream << "\n";
        //get matrices on bus

        for(auto mtxId: std::as_const(mtxNames))
        {
            auto mtx = mtxMgr->router(mtxId);
            if (mtx->busId()!=busId)
                continue;
            stream << "    ";
            stream << "Router "<< mtxId<<" ";
            if (!mtx)
            {
                stream << "not loaded.\n";
                continue;
            }
            stream << " "
                   << mtx->numSources()
                   << "x"
                   << mtx->numDestinations()
                   << " @"<<mtx->busAddr() <<":"<<mtx->level();
            stream <<"\n";
        }
        if (mtxNames.isEmpty())
            stream << "  (empty)\n";
        stream.flush();
    }

}



int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

    qInstallMessageHandler(debugMessageHandler);

    qInfo()<<"Starting up";
    Router::BusManager::inst();

    HttpServer srv(&a);
    staticFileServer fs(":/webui", &a);
    srv.registerHandler("/", &fs);
    REST::Controller rsc(&a);
    REST::QHttpServerAdapter adapter(QPointer<REST::Controller>(&rsc), &a);



    rsc.registerRessource("/router/", new RouterBaseRessource(&rsc, &a));
    rsc.registerRessource("/bus/", nullptr);
    rsc.registerRessource("/panel/", nullptr);
    QString apiBase="";

    for (auto path: rsc.urlMatches())
        srv.registerHandler(path, &adapter, true);

    qInfo()<<"Loading configuration";

    QFile cfgfile("/home/andreas/cfg_routerctl.json");
    cfgfile.open(QFile::ReadOnly);
    QJsonParseError err;
    QByteArray cfgData = cfgfile.readAll();

    QJsonDocument jd = QJsonDocument::fromJson(cfgData, &err);

    if (err.error != QJsonParseError::NoError)
    {
        qCritical()<<"Config file could not be parsed: " << err.errorString()<<err.offset;
        qDebug()<<"Context: " << cfgData.mid(err.offset);
//        return 1;
    }
    else
    {
    }
    qDebug()<<jd.object().keys();
    QJsonObject busses = jd.object().value("router-busses").toObject();
    if (busses.isEmpty())
    {
        qCritical()<<"No matrix busses were defined.";
        qInfo()<<"No interactive configuration possible yet. terminating.";
        return 1;
    }
    if (!Router::BusManager::inst()->load(busses))
    {
        qCritical()<<"could not load any busses, see error messages for details";
    }
    QJsonObject routers = jd.object().value("routers").toObject();
    if (routers.isEmpty())
    {
        qCritical()<<"No matrix busses were defined.";
        qInfo()<<"No interactive configuration possible yet. terminating.";
        return 1;
    }
    Router::MatrixManager::inst()->load(routers);

    QTextStream stream(stdout);
    showConfig(stream);
    return a.exec();
}
