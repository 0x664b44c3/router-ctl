#ifndef ROUTERWRAPPER_H
#define ROUTERWRAPPER_H

#include <QObject>
#include "engine_common.h"


class RouterWrapper : public QObject
{
	Q_OBJECT
public:
	explicit RouterWrapper(QObject *parent = nullptr);
	//QList<RouterEndpoint

signals:

public slots:
	void setRoute(QString destination, QString source);
};

#endif // ROUTERWRAPPER_H
