#ifndef VIDEOHUBSERVER_H
#define VIDEOHUBSERVER_H

#include <QObject>

class VideoHubServer : public QObject
{
	Q_OBJECT
public:
	explicit VideoHubServer(QObject *parent = nullptr);

signals:

public slots:
};

#endif // VIDEOHUBSERVER_H