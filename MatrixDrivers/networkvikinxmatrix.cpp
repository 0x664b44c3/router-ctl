#include "networkvikinxmatrix.h"
#include <QIODevice>
#include <QElapsedTimer>
#include <QVector>
#include <QDebug>
#include <QTimer>

#define LINK_TIMEOUT 1000000l

enum NevionCommands
{
	cmdAudioXPointSet   = 0x90,
	cmdVideoXPointSet   = 0xa0,
	cmdXPointStatus     = 0xc0,
	cmdXPointLockOutput = 0xe0,
	cmdXPointRouterReq  = 0xdf
};

enum RouterRequestDataType
{
	reqInput  = 0,
	reqOutput = 1,
	reqFormat = 2,
	reqRequest=3
};

enum parserStatus
{
	psReset=0,
	psXpointVideoDestination=1,
	psXpointVideoSource = 2,
	psXpointAudioDestination=1,
	psXpointAudioSource = 2,
};

NetworkVikinxMatrix::NetworkVikinxMatrix(int level, QObject *parent) :
    AbstractVideoMatrix(parent),
    mOutputs(0),
    mTimeOutTimer(QElapsedTimer()),
    mParserStatus(0),
    mLevel(level),
    mDataChannel(NULL)
{
	mOutputAssignVideo.resize(0);
	for(int i=0;i<128;++i)
		mOutputAssignVideo.append(-1);
//	QTimer * tmr = new QTimer(this);
//	tmr->start(500);
//	connect(tmr, SIGNAL(timeout()), SLOT(test()));

}

int NetworkVikinxMatrix::sourceCount() const
{
	return 128;
}

int NetworkVikinxMatrix::destinationCount() const
{
	return mOutputs;
}

int NetworkVikinxMatrix::sourceForDestination(int output) const
{
	if ((output < 0) || (output >= mOutputs))
		return -1;
	return mOutputAssignVideo.at(output);
}

bool NetworkVikinxMatrix::setSource(int output, int input)
{
	return execXPoint(output, input);
}

bool NetworkVikinxMatrix::isOnline() const
{
	return false;
}

void NetworkVikinxMatrix::setIODevice(QIODevice * dev, bool devIsShared)
{
	if (mDataChannel)
	{
		mDataChannel->disconnect();
	}
	mDataChannel = dev;
	if (!devIsShared)
		mDataChannel->setParent(this);
	connect(dev, SIGNAL(readyRead()), SLOT(onRxData()));
	queryStatus();
}

void NetworkVikinxMatrix::assign(int output, int input)
{
	setSource(output, input);
}

void NetworkVikinxMatrix::test()
{
	for (int i=0;i<32;++i)
		setSource(i, i);
}

bool NetworkVikinxMatrix::execXPoint(int out, int in)
{
	if (!mDataChannel)
		return false;
	QByteArray packet;
	packet.push_back(cmdVideoXPointSet | (mLevel & 0x0f));
	packet.push_back(out & 0x7f);
	packet.push_back(in  & 0x7f);
	mDataChannel->write(packet);
	return true;
}

void NetworkVikinxMatrix::queryStatus()
{
	if (!mDataChannel)
		return;
	QByteArray packet;
	packet.push_back(cmdXPointStatus | (mLevel & 0x0f));
	packet.push_back((char) 0x01);
	mDataChannel->write(packet);
}

void NetworkVikinxMatrix::handleXPointChange(int out, int in)
{
	if (out >= mOutputs)
	{
		mOutputs = out + 1;
		mOutputAssignVideo.resize(mOutputs);
	}
	int oldInput = mOutputAssignVideo[out];
	mOutputAssignVideo[out] = in;

	qDebug()<<"out " << out << " <== " << in;
	if (oldInput != in)
		emit outputChanged(out, in);
}

void NetworkVikinxMatrix::doRouterRequest()
{
	if (!mDataChannel)
		return;
	bool requestAudio = false;
	QByteArray packet;
	packet.push_back(cmdXPointRouterReq);
	packet.push_back(
	              (reqRequest << 5)
	            | (requestAudio ? 0x40 : 0x00)
	            | (mLevel & 0x0f)
	            );
	packet.push_back((char) 0);
	mDataChannel->write(packet);
}

void NetworkVikinxMatrix::parseCommData(const QByteArray & rxData)
{
	qint64 elapsed = mTimeOutTimer.restart();
	if (elapsed > LINK_TIMEOUT)
		mParserStatus = 0;
	foreach(unsigned char c, rxData)
	{
		if (c & 0x80)
		{
			int level = c & 0x0f;
			switch(c & 0xf0)
			{
				case cmdAudioXPointSet:
					if (level == mLevel)
						mParserStatus = psXpointAudioDestination;
					else
						mParserStatus = psReset;
					break;
				case cmdVideoXPointSet:
					if (level == mLevel)
						mParserStatus = psXpointVideoDestination;
					else
						mParserStatus = psReset;
					break;
				default:
					mParserStatus = psReset;
					break;
			}
		}
		else
		{
			switch(mParserStatus)
			{
				case psXpointVideoDestination:
					mParameterBuffer[0] = c & 0x7f;
					mParserStatus = psXpointVideoSource;
					break;
				case psXpointVideoSource:
					mParameterBuffer[1] = c & 0x7f;
					handleXPointChange(mParameterBuffer[0], mParameterBuffer[1]);
					mParserStatus = psReset;
					break;
				default:
					mParserStatus = psReset;
					break;
			}
		}
	}
}

void NetworkVikinxMatrix::onRxData()
{
	while(!mDataChannel->atEnd())
	{
		QByteArray input = mDataChannel->readAll();
		qDebug()<<"rx"<<input;
		qDebug()<< input;
		parseCommData(input);
	}
}
