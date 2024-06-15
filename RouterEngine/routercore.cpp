#include "routercore.h"
#include <QDebug>
#include <iomanip>
#include <iostream>

using namespace Router;

bool Router::dirIsValid(Port::Direction d) {
    if ((d == Port::Direction::Source) || (d == Port::Direction::Destination))
        return true;
    return false;
}

void recurseLinks(const QMap<QString, QStringList> map, QString master, int indent=0)
{
    QString output;
    if (indent>10)
    {
        qDebug() << "(!!!) recursion limit hit";
        return;
    }

    for(int i=0;i<indent;++i)
        output += "  ";
    output += master;
    qDebug().noquote() << output;

    QStringList linkList = map.value(master);
    foreach(QString linked, linkList)
    {
        recurseLinks(map, linked, indent+1);
    }
}



void RouterCore::debugLinks()
{
    //FIXME: redo this once linked ports have been refactored
    // qDebug().noquote() << "Linked ports:";
    // QStringList rootlevel;
    // foreach (QString port, mLinkedPorts.keys()) {
    //     if (mLinkedPorts.values().contains(port))
    //         continue;
    //     if (rootlevel.contains(port))
    //         continue;
    //     rootlevel << port;
    //     recurseLinks(mLinkedPorts, port, 0);
    // }
    // qDebug() << "";
}

QDebug operator<<(QDebug debug, const Router::Request &req)
{
    QDebugStateSaver saver(debug);
    debug.nospace() <<"{rtr:"<<req.routerUID<<", "
                    << req.destinationUID
                    << " <-- " << req.sourceUID
                    << "}";
    return debug;
}


RouterCore::RouterCore(QObject *parent) : QObject(parent)
{

}

Router::Endpoint RouterCore::findEndpoint(QString endpointID, Port::Direction dir) const
{
    Router::Endpoint result = mPorts.value(endpointID);
    if (result.direction != dir)
        result.clear();
    return result;
}


QList<Router::Request> RouterCore::findRoute(QString destinationId, QString sourceID, int ttl) const
{
    QList<Router::Request> route;
    if (ttl<0)
        return route;
    Router::Endpoint dstEP = findEndpoint(destinationId, Port::Direction::Destination);
    Router::Endpoint srcEP = findEndpoint(sourceID, Port::Direction::Source);
    if ((dstEP.isNull()) || srcEP.isNull())
    {
        qDebug()<<"source or dest unknown";
        return route;
    }
    if (dstEP.routerUID == srcEP.routerUID)
    {
        Router::Request req;
        req.routerUID = dstEP.routerUID;
        req.sourceUID = sourceID;
        req.destinationUID = destinationId;
        req.setLocks.clear();
        route << req;
    }
    else
    {
        //performs a one-level tie line search
        // hopefully i can reuse some code later
        //TODO: implement dijkstra or some breadth-first search here
        //then split this out to first find the route, then create the routing commands

        //final algo: try one-level search first, if a direct link can be established, do so (computationally cheap)
        //check all possible links and select the cheapest one
        //once we have a prospective route (or not) try
        //running dijkstra anyway to see if another tie-route is cheaper


        //simple one-level tie line search is below

        //the router we're at right now
        QString currentRouter = dstEP.routerUID;

        foreach(auto tie, mTieLines)
        {
            //skip ties that do not go into this router
            if (tie.destinationRouterID != currentRouter)
                continue;

            //basically here we need to recurse
            if (tie.sourceRouterID != srcEP.routerUID)
                continue;
            // a source to a tie is destination on a router
            Router::Endpoint tieSource = findEndpoint(tie.sourcePortID, Port::Direction::Destination);
            //source port not defined, skip this one; bad data
            if (tieSource.isNull())
                continue;
            //if the tie source port is already locked by another destination skip this as well
            if (isDestinationLocked(tieSource, destinationId))
                continue;

            //routing request assigning the tie port at the source router
            Router::Request tieRequest;
            tieRequest.routerUID = tie.sourceRouterID;
            tieRequest.sourceUID = sourceID;
            tieRequest.destinationUID = tie.sourcePortID;
            tieRequest.setLocks << destinationId;
            route << tieRequest;

            //routing request on destination router
            Router::Request mainRequest;
            mainRequest.routerUID = dstEP.routerUID;
            mainRequest.sourceUID = tie.destinationPortID;
            mainRequest.destinationUID = destinationId;
            mainRequest.setLocks.clear();
            route << mainRequest;

            break;
        }
        qDebug()<<"FIXME: Cannot really route between matrices in this version, only halve-arsed implementation ATM";
    }

    QStringList linkedPorts = mLinkedPorts.keys(destinationId);

    if (ttl>0)
    {
        foreach(QString linkedDest, linkedPorts)
            route << findRoute(linkedDest, sourceID, ttl - 1);
    }
    else
    {
        qDebug()<< "maximum route search depth exceeded";
    }

    return route;
}

//FIXME: rework this, should remove all ports linked to this one
void RouterCore::clearLinksForTarget(QString target)
{
    // mLinkedPorts
}

bool RouterCore::isDestinationLocked(Router::Endpoint ep, QString uid) const
{
    return isDestinationLocked(ep, QStringList()<<uid);
}

bool RouterCore::isDestinationLocked(Router::Endpoint ep, QStringList excludeUIDs) const
{
    if (ep.lockedBy.isEmpty())
        return false;
    bool linkedPortsFound = true;
    int rounds=10; //prevent endless loops if data corrupt (loops in link table)
    while(linkedPortsFound)
    {
        if  (rounds<=0)
        {
            qDebug()<<"link search took too many iterations - recursion in loop table?";
            break;
        }
        linkedPortsFound = false;
        foreach (QString id, excludeUIDs)
        {
            QStringList linkedPorts = mLinkedPorts.keys(id);
            foreach(QString port, linkedPorts)
            {
                if (excludeUIDs.contains(port))
                    continue;
                linkedPortsFound = true;
                excludeUIDs.push_back(port);
            }
        }
    }
    QStringList locks = ep.lockedBy;
    foreach (QString id, excludeUIDs)
        locks.removeAll(id);
    if (ep.lockedBy.isEmpty())
        return false;
    return true;
}

int RouterCore::clearLocks(QString endpoint, QString locker)
{
    if (mPorts.contains(endpoint))
    {
        Router::Endpoint & ep = mPorts[endpoint];
        return ep.lockedBy.removeAll(locker);
    }
    return 0;
}

/** use dijkstra's algorithm here*/
void RouterCore::findBestRoute(QString destinationRouter, QString sourceRouter)
{


}

void RouterCore::setLock(QString endpoint, QString locker)
{
    if (mPorts.contains(endpoint))
    {
        Router::Endpoint & ep = mPorts[endpoint];
        ep.lockedBy << locker;
    }
}

QMap<QString, Router::Endpoint> RouterCore::destinations() const
{
    QMap<QString, Router::Endpoint> ret;
    for(auto it = mPorts.begin(); it!=mPorts.end();++it)
    {
        if (it->direction == Port::Direction::Destination)
            ret.insert(it.key(), *it);
    }
    return ret;
}

QStringList RouterCore::sourceUIDs() const
{
    return sources().keys();
}

QStringList RouterCore::destinationUIDs() const
{
    return destinations().keys();
}

QStringList RouterCore::traceLinkChain(QString destination, int maxRecursion)
{
    QStringList chain;
    if (maxRecursion<0)
    {
        qDebug()<<"Warning: recursion limit reached during backtracking linked ports";
        return chain;
    }

    //NOTE: the list of port links should be inverted as any port can only be linked to a single other port
    //this avoids all the mess currently in here with string lists etc
    //FIXME: this is non-functional for the time being
    QString master ; //= mLinkedPorts.key(destination);
    if (master.isEmpty())
        return chain;
    chain << master;
    chain << traceLinkChain(master, maxRecursion-1);
    return chain;
}

bool RouterCore::uidExists(QString uid) const
{
    return sourceUIDs().contains(uid) || destinationUIDs().contains(uid);
}

void RouterCore::reset()
{
    mPorts.clear();
    mConnectionTable.clear();
    mLinkedPorts.clear();
}

void RouterCore::testcase()
{
    reset();
    for (int j=0;j<4; ++j)
    {
        Router::Info rtr;
        rtr.nInputs  = 32;
        rtr.nOutputs = 32;
        rtr.uid      = QString("test-matrix-%1").arg(j);
        for (int i=0;i<32;++i)
        {
            {
                Router::Endpoint ep(Port::Direction::Source, rtr.uid, i);
                ep.mnemonic = QString("Input %1.%2").arg(j).arg(i);
                mPorts.insert(QString("in-test-%1.%2").arg(j).arg(i), ep);
            }
            {
                Router::Endpoint ep(Port::Direction::Destination, rtr.uid, i);
                ep.mnemonic = QString("Output %1.%2").arg(j).arg(i);
                mPorts.insert(QString("out-test-%1.%2").arg(j).arg(i), ep);
            }
        }
    }
    qDebug()<<"Sources:"<<sourceUIDs();
    qDebug()<<"Destinations:"<<destinationUIDs();

    requestRoute("out-test-0.1", "in-test-0.1");
    requestRoute("out-test-0.2", "in-test-0.4");
    debugLinks();
    requestRoute("out-test-0.3", "out-test-0.1");
    debugLinks();
    requestRoute("out-test-0.4", "out-test-0.3");
    debugLinks();
    requestRoute("out-test-5", "out-test-1");
    debugLinks();
    qDebug()<<"switching destination with linked ports";
    //    qDebug()<<mConnectedSources;
    requestRoute("out-test-0.1", "in-test-0.8");
    debugLinks();
    //    qDebug()<<mConnectedSources;
    debugLinks();
    requestRoute("out-test-0.3", "in-test-0.10");
    //    qDebug()<<mConnectedSources;
    debugLinks();
    requestRoute("out-test-0.3", "out-test-0.4");
    debugLinks();
    qDebug()<<"connections"<<mConnectionTable;
    //tie line test
    //two ties bidirectional between router 0 and 1
    registerTie("out-test-0.16", "in-test-1.16");
    registerTie("out-test-1.16", "in-test-0.16");
    registerTie("out-test-0.17", "in-test-1.17");
    registerTie("out-test-1.17", "in-test-0.17");


    registerTie("out-test-1.0", "in-test-2.0");
    registerTie("out-test-1.1", "in-test-2.1");

    registerTie("out-test-2.0", "in-test-0.18");
    registerTie("out-test-2.1", "in-test-0.19");

    requestRoute("out-test-0.4", "in-test-1.3");
    requestRoute("out-test-0.4", "in-test-1.4");
    requestRoute("out-test-0.4", "in-test-0.4");
    requestRoute("out-test-0.4", "in-test-0.5");
}

bool RouterCore::registerTie(QString sourcePort, QString destPort, int penalty)
{
    //source/destination are swapped here
    Router::Endpoint srcEP = findEndpoint(sourcePort, Port::Direction::Destination);
    Router::Endpoint dstEP = findEndpoint(destPort, Port::Direction::Source);
    if ((srcEP.isNull()|| dstEP.isNull()))
        return false;
    //ties on the same router make no sense
    if (srcEP.routerUID == dstEP.routerUID)
        return true;
    Router::TieLine tie;
    tie.destinationPortID = destPort;
    tie.sourcePortID = sourcePort;
    tie.destinationRouterID = dstEP.routerUID;
    tie.sourceRouterID = srcEP.routerUID;
    tie.penalty = penalty;
    mTieLines.push_back(tie);
    return true;
}
void RouterCore::requestRoute(QString destination, QString source)
{
    qDebug() << "Request route from source" << source
             << "to destination" << destination;
    QList<Router::Request> route;
    Router::Endpoint dstEP = findEndpoint(destination, Port::Direction::Destination);
    if (dstEP.isNull())
    {
        qDebug()<<"Destination"<<destination<<"not found";
        return;
    }
    //any new route will break an active link
    mLinkedPorts.remove(destination);

    if (destinationUIDs().contains(source))
    {
        Router::Endpoint srcEP = findEndpoint(source, Port::Direction::Destination);
        if (srcEP.isNull())
        {
            qDebug()<<"Destination"<<source<<"unknown while trying to link";
            return;
        }

        //record link
        QStringList parentLinks = traceLinkChain(source);
        if (parentLinks.contains(destination))
        {
            qDebug()<<"Not creating link (recursion detected):"
                     << "Destination: " << destination
                     <<  "Source: " << source;
        }
        else
        {
            //FIXME: rewrite this to be the opposite way round
            //mLinkedPorts.insertMulti(source, destination);
            qDebug()<<"Destination" << destination
                     << "now linked to" <<source;
            QString effectiveSourceID = mConnectionTable.value(source);
            route = findRoute(destination, effectiveSourceID);
        }
    }
    else
    {
        route = findRoute(destination, source);
    }

    if (route.isEmpty())
    {
        qDebug()<<"No route found to connect source"<<source
                 <<"to destination"<<destination;
    }
    else
    {
        foreach (Router::Request req, route)
        {
            //add locking/unlocking here
            //			RouterEndpoint ep = findEndpoint(req.sourceUID, dirSource);
            //			if (!ep.isNull())
            //			{
            //				qDebug()<< clearLocks(req.sourceUID, req.destinationUID)
            //				        << "locks were removed";
            //			}
            //			if (req.setLocks.size())
            //			{
            //				setLock(req.destinationUID, req.sourceUID);
            //			}
            mConnectionTable.insert(req.destinationUID, req.sourceUID);
        }
        qDebug()<<"routes"<<route;
    }

}

QMap<QString, Router::Endpoint> RouterCore::sources() const
{
    QMap<QString, Router::Endpoint> ret;
    for(auto it = mPorts.begin(); it!=mPorts.end();++it)
    {
        if (it->direction == Port::Direction::Source)
            ret.insert(it.key(), *it);
    }
    return ret;
}
