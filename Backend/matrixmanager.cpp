#include "matrixmanager.h"
#include <QCoreApplication>

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

MatrixManager::MatrixManager(QObject *parent)
    : QObject{parent}
{

}

void MatrixManager::onXPointChanged(QString busId, int addr, int level, int dst, int src)
{

}

} // namespace Router
