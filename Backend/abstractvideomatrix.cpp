#include "abstractvideomatrix.h"

AbstractVideoMatrix::AbstractVideoMatrix(QObject *parent) : QObject(parent)
{

}

QString AbstractVideoMatrix::uid() const
{
	return mUID;
}

void AbstractVideoMatrix::setUid(QString id)
{
	mUID = id;
	emit uidChanged(mUID);
}
