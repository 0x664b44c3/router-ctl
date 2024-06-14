#ifndef GLOBAL_H
#define GLOBAL_H
#include <QtGlobal>
#include <QStringList>
#include <QDebug>


namespace Router {

namespace Port {
    enum class Direction
    {
        UnspecifiedDir=0,
        Source,
        Destination
    };
    struct Info
    {
        Direction direction;
        QString uid;
        QString mnemonic;
    };
}

    bool dirIsValid(Port::Direction d);

struct Endpoint
{

public:
    Endpoint() {clear();}
    Endpoint(Port::Direction dir, QString router, int port)
    {
        clear();
        direction  = dir;
        routerUID  = router;
        portNumber = port;
    }
    bool isNull() const
    {
		if ((routerUID.isNull()) || (portNumber < 0))
            return true;
        return false;
    }
    void clear()
    {
        routerUID.clear();
        mnemonic.clear();
        shortMnemonic.clear();
        portNumber = -1;
        direction = Port::Direction::Source;
		lockedBy.clear();
        tallyBits=0;
    }
    Port::Direction direction;
    QString routerUID;
    QString mnemonic;
    QString shortMnemonic;
	QStringList lockedBy;
    int portNumber;
    int tallyBits;
};

struct Info
{
    QString uid;
    int nInputs;
    int nOutputs;
};

struct TieLine
{
	QString uid;

	QString sourceRouterID;
    QString destinationRouterID;
	QString sourcePortID;
	QString destinationPortID;
	//cost to make this tie, default is 100
	int penalty;
};

struct Request
{
    QString routerUID;
    QString sourceUID;
    QString destinationUID;
	QStringList setLocks;
};
}

#endif // GLOBAL_H
