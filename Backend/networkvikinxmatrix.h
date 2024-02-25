#ifndef NETWORKVIKINXMATRIX_H
#define NETWORKVIKINXMATRIX_H

#include <QObject>
#include <QElapsedTimer>
#include <QVector>
#include "abstractvideomatrix.h"

QT_BEGIN_NAMESPACE
class QIODevice;

QT_END_NAMESPACE

class NetworkVikinxMatrix : public AbstractVideoMatrix
{
	Q_OBJECT
public:
	NetworkVikinxMatrix(int level, QObject * parent = 0);


	virtual int sourceCount() const;
	virtual int destinationCount() const;
	virtual int sourceForDestination(int output) const;
	virtual bool setSource(int output, int input);
	virtual bool isOnline() const;

	void setIODevice(QIODevice *, bool devIsShared = false);
	void queryStatus();

public slots:
	void assign(int output, int input);
	void test();
private:
	bool execXPoint(int out, int in);
	void handleXPointChange(int out, int in);
	/**
	 * @brief doRouterRequest
	 * Queries the NCB network for attached routers, making each router reply
	 * with its size and format capabilities
	 */
	void doRouterRequest();
	int mOutputs;
	QElapsedTimer mTimeOutTimer;
	int mParserStatus;
	int mParameterBuffer[2];
	int mLevel;
	QVector<int> mOutputAssignVideo;
	QIODevice * mDataChannel;

	void parseCommData(const QByteArray &);
private slots:
	void onRxData();

};

#endif // NETWORKVIKINXMATRIX_H
