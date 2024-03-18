#include "matrix.h"

namespace Router {

Matrix::Matrix(QObject *parent)
    : QObject{parent}
{}

int Matrix::level() const
{
    return mLevel;
}

void Matrix::setLevel(int newLevel)
{
    mLevel = newLevel;
}

int Matrix::frameId() const
{
    return mFrameId;
}

void Matrix::setFrameId(int newFrameId)
{
    mFrameId = newFrameId;
}

void Matrix::setXPoint(int dest, int source)
{

}

void Matrix::setLabel(Port::Direction dir, int id, QString label)
{

}

void Matrix::lockPort(int dest)
{

}

void Matrix::unlockPort(int dest)
{

}

QString Matrix::busId() const
{
    return mBusId;
}

void Matrix::setBusId(const QString &newBusId)
{
    mBusId = newBusId;
}

} // namespace Router
