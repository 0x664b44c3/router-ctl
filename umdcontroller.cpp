#include "umdcontroller.h"
#include <QDateTime>
#include <QMap>
#include <QDebug>
UMDController::UMDController(QObject *parent) : QObject(parent)
{

}

bool UMDController::loadConfig(QJsonObject jObj)
{

	return false;
}

QString UMDController::formatUMD(QString text)
{
	QMap<QString, QString> variables;
	QDateTime time = QDateTime::currentDateTime();
	variables.insert("time", time.toString("hh:mm:ss"));
	variables.insert("hour", time.toString("hh"));
	variables.insert("minute", time.toString("mm"));
	variables.insert("second", time.toString("ss"));
	time = QDateTime::currentDateTimeUtc();
	variables.insert("time_utc", time.toString("hh:mm:ss"));
	variables.insert("hour_utc", time.toString("hh"));
	variables.insert("minute_utc", time.toString("mm"));
	variables.insert("second_utc", time.toString("ss"));
	qDebug()<<variables;
	int start=0;
	QString out = text;
	bool inVar=false;
	int i = out.indexOf('%', start);
	while (i>-1)
	{
//		qDebug()<<inVar<<start<<i<<out;
		if (inVar)
		{
			QString varName = out.mid(start, i-start);
			if (varName.isEmpty()) //double % becomes single %
			{
				out = out.left(start)
				      + out.mid(i+1);
				start = start + 1;
			}
			else
			{
				QString value = variables.value(varName, QStringLiteral("VAR_ERROR"));
				out =   out.left(start-1)
				        + value
				        + out.mid(i+1);
				start = start + value.length();

			}
			inVar = false;
		}
		else
		{
			start=i+1;
			inVar = true;
		}
		i = out.indexOf('%', start);
	}
	return out;

}

void UMDController::destinationChanged(QString destinationUID)
{

}
