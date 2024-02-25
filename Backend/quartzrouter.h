#ifndef QUARTZROUTER_H
#define QUARTZROUTER_H

#include <QObject>
#include <QIODevice>
class QuartzRouter : public QObject
{
Q_OBJECT

public:
	explicit QuartzRouter(QObject * parent =nullptr);

	int sourceForDestination(int output) const;
	bool setSource(QString dest, QString source);
	bool setSource(int dest, int source);
	bool isOnline() const;
	int numSources() const;
	int numDestinations() const;

private:

	void sendSetXpt(QString level, int dest, int src);
	void requestSource(int destination);
	QString mLevels;
	int mNumDestinations;
	int mNumSources;
	QList<QString> mInputUIDs;
	QList<QString> mOutputUIDs;
	QIODevice * mChannel;
	QByteArray mBuffer;
	void processData();
	void processResponse(QString cmd);

public slots:

};

#endif // QUARTZROUTER_H
