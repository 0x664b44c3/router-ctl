#include <QCoreApplication>
//#include <webserver.h>
#include "videomatrixemu.h"
//#include "networkvikinxmatrix.h"
#include <QSerialPort>
#include <QDebug>
#include <QTimer>

#include "routercore.h"
#include "quartzrouter.h"
#include <umdcontroller.h>

#include "leitchbusdriver.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>

#include <busmanager.h>
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




int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

    qInstallMessageHandler(debugMessageHandler);

    RouterCore core;
//	QuartzRouter quartz;

//	Webserver ws(&a);
//	ws.setStaticPath(":/wwwdata/");
//	ws.setIndexDoc("index.htm");
//	NetworkVikinxMatrix mtx(0, &a);

	QSerialPort ser("/dev/ttyUSB4", &a);
	ser.setBaudRate(19200);
	if (!ser.open(QIODevice::ReadWrite))
	{
		qDebug()<<"Could not open port";
	}
//	mtx.setIODevice(&ser);
//	mtx.queryStatus();
//	QTimer t;
//	t.setInterval(100);
////		t.start();
//	QObject::connect(&t, SIGNAL(timeout()), &mtx, SLOT(test()));

//	for (int i=0;i<16;++i)
//		mtx.assign(i, i);

//	return a.exec();
    //core.testcase();
    //qDebug()<<UMDController::formatUMD("%time% has no extra %% in it sinc %hour%:%minute%");

    qInfo()<<"Starting up";
    Router::BusManager::inst();


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
//        return 1;
    }
    Router::BusManager::inst()->load(busses);



    return a.exec();
}
