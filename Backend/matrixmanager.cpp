#include "matrixmanager.h"
#include <QCoreApplication>
#include "busmanager.h"
//initialize singleton instance ptr
Router::MatrixManager * Router::MatrixManager::mInst = nullptr;


namespace Router {

MatrixManager *MatrixManager::inst()
{
    if (!mInst)
    {
        mInst = new MatrixManager(QCoreApplication::instance());
    }
    return mInst;
}

bool MatrixManager::load(const QJsonObject & cfg)
{
    qDebug()<<cfg;
    reset();
    for(auto it = cfg.begin(); it != cfg.end(); ++it)
    {
        QString routerId = it.key();
        if (mRouters.contains(routerId))
        {
            qWarning()<<"Router with id"<<routerId<<"already present in config";
            continue;
        }
        QJsonValue mcfg = it.value();
        if (!mcfg.isObject())
        {
            qWarning()<<"Router cfg for" << routerId << "not an object";
            continue;
        }
        auto mtx = new Matrix(routerId, this);
        if (!mtx->loadConfig(mcfg.toObject()))
        {
            qWarning()<<"Router cfg for" << routerId << "invalid. Not creating matrix";
            //TODO: set an error/alarm code here
            mtx->deleteLater();
            continue;
        }
        mRouters.insert(routerId, mtx);
    }
    return true;
}

int MatrixManager::alarms() const
{
    int alm=0;
    for(auto rtr: std::as_const(mRouters))
        alm|=rtr->alarms();
    return alm;
}

QStringList MatrixManager::getRouterIds() const
{
    return mRouters.keys();
}

Matrix *MatrixManager::router(QString uid)
{
    return mRouters.value(uid, nullptr);
}

void MatrixManager::reset()
{
    for(auto it = mRouters.begin(); it != mRouters.end(); ++it)
    {
        emit routerDeleted(it.key());
        it.value()->deleteLater();
    }

}

MatrixManager::MatrixManager(QObject *parent)
    : QObject{parent}
{
    connect(BusManager::inst(), &BusManager::xPointChanged, this, &MatrixManager::onXPointChanged);
    // auto rtr = new Matrix("test-router", this);
    // rtr->resize(32, 32);
    // mRouters.insert("test-router", rtr);
}

void MatrixManager::onXPointChanged(QString busId, int addr, int level, int dst, int src)
{
    for(auto & rtr: mRouters)
    {
        if ((rtr->busId() == busId) && (rtr->busAddr() == addr))
        {
            rtr->onXPointChanged(busId, addr, level, dst, src);
        }
    }
}

} // namespace Router
