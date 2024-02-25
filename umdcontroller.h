#ifndef UMDCONTROLLER_H
#define UMDCONTROLLER_H

#include <QObject>
#include <QJsonObject>

class UMDController : public QObject
{
	Q_OBJECT
public:
	explicit UMDController(QObject *parent = nullptr);
	bool loadConfig(QJsonObject jObj);
	static QString formatUMD(QString text);

signals:


public slots:
	void destinationChanged(QString destinationUID);
protected:
	int mRepeatTimeout;
	bool mDebug;
};

#endif // UMDCONTROLLER_H
