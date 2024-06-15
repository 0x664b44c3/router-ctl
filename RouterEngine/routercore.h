#ifndef ROUTERCORE_H
#define ROUTERCORE_H

#include <QObject>
#include <engine_common.h>
#include <QMap>
#include <QList>




class RouterCore : public QObject
{
    Q_OBJECT
public:
    explicit RouterCore(QObject *parent = nullptr);

    QMap<QString, Router::Endpoint> sources() const;
    QMap<QString, Router::Endpoint> destinations() const;
    QStringList sourceUIDs() const;
    QStringList destinationUIDs() const;
    QStringList traceLinkChain(QString destination, int maxRecursion=10);
    bool uidExists(QString uid) const;

    void reset();
    void testcase();
	void debugLinks();
	bool registerTie(QString sourcePort, QString destPort, int penalty = 100);
public slots:
    void requestRoute(QString destination, QString source);

private:
    Router::Endpoint findEndpoint(QString endpointID, Router::Port::Direction dir) const;
    QList<Router::Request> findRoute(QString destinationID, QString sourceID, int ttl = 4) const;
    QList<Router::Request> applyLinkedPorts(QList<Router::Request>);
    void clearLinksForTarget(QString target);
	/**
	 * @brief isDestinationLocked
	 * @param ep endpoint info
	 * @param excludeUID return false if the port is only locked by these UIDs
	 * @return
	 */

signals:
    void routeEstablished(QString destination, QString source);

protected:
    QList<Router::Info> mRouters;
    QMap<QString, Router::Endpoint> mPorts;
	QMap<QString, QString> mConnectionTable;
    QList<Router::TieLine> mTieLines;

    //holds a map of [port]=>[linked to], when a port in a value changes, the port in key also is changed
    QMap<QString, QString> mLinkedPorts;

    bool isDestinationLocked(Router::Endpoint ep, QString excludeUID) const;
    bool isDestinationLocked(Router::Endpoint ep, QStringList excludeUIDs=QStringList()) const;
	void setLock(QString endpoint, QString locker);
	int clearLocks(QString endpoint, QString locker);
	void findBestRoute(QString destinationRouter, QString sourceRouter);
};

#endif // ROUTERCORE_H
