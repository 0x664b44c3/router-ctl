#include "quartzrouter.h"


QuartzRouter::QuartzRouter(QObject *parent) :
    QObject(parent),

    mLevels(QStringLiteral("V")),
    mNumDestinations(32),
    mNumSources(32)

{

}

int QuartzRouter::sourceForDestination(int output) const
{

}

bool QuartzRouter::setSource(QString dest, QString source)
{

}

bool QuartzRouter::setSource(int dest, int source)
{
	sendSetXpt(mLevels, dest, source);
}

bool QuartzRouter::isOnline() const
{

}

int QuartzRouter::numSources() const
{
	return mNumSources;
}

int QuartzRouter::numDestinations() const
{
	return mNumDestinations;
}

void QuartzRouter::sendSetXpt(QString level, int dest, int src)
{
	if (src<0)
		return;
	if (src>=numSources())
		return;
	if (dest<0)
		return;
	if (dest>=numDestinations())
		return;
	QString cmd =
	        ".S" + level +
	        QString::asprintf("%d,%d\r", dest, src);
	mChannel->write(cmd.toLatin1());
}

void QuartzRouter::requestSource(int destination)
{
	if (destination<0)
		return;
	if (destination>=numDestinations())
		return;
	QString cmd =
	        ".I" + mLevels +
	        QString::asprintf("%d\r", destination);
	mChannel->write(cmd.toLatin1());
}

void QuartzRouter::processData()
{
	if (mBuffer.isEmpty())
		return;
	int start = 0;
	int eol = -1;
	while(1)
	{
		eol = mBuffer.indexOf('\r', start);
		if (eol<0)
			break;
		if ((start-eol)>0)
			processResponse(QString::fromLatin1(mBuffer.mid(start, eol-1)));
		start=eol+1;
	}
	mBuffer = mBuffer.mid(start);
}

void QuartzRouter::processResponse(QString cmd)
{
//	if (cmd.startsWith("."))
}
