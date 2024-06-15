#include "mimedb.h"
#include <QHash>
MimeDB *MimeDB::mInst = nullptr;

MimeDB::MimeDB(const QString defaultMIME) :
    mDefaultMimeType(defaultMIME)
{
    setupDefaultTypes();
}

QString MimeDB::defaultMimeType() const
{
    return mDefaultMimeType;
}

void MimeDB::addExtension(QString ext, QString mime)
{
    mTypes[ext] = mime;
}

QString MimeDB::mime(QString fileName) const
{
	int loc = fileName.lastIndexOf('.') + 1;
	if ((loc<0) || (loc>=fileName.length()))
        return mDefaultMimeType;
	QString ext = fileName.mid(loc);
    return mTypes.value(ext, mDefaultMimeType);
}

void MimeDB::setDefaultMimeType(QString s)
{
    mDefaultMimeType = s;
}

void MimeDB::setupDefaultTypes()
{
	addExtension("png", "image/png");
	addExtension("jpg", "image/jpeg");
	addExtension("jpeg", "image/jpeg");
	addExtension("htm", "text/html; charset=UTF-8");
	addExtension("html", "text/html; charset=UTF-8");
	addExtension("cgi", "text/html; charset=UTF-8");
    addExtension("css", "text/css; charset=UTF-8");

    addExtension("js", "text/javascript; charset=UTF-8");
    addExtension("json", "application/json; charset=UTF-8");

}

MimeDB *MimeDB::inst()
{
    if (!mInst)
        mInst = new MimeDB();
    return mInst;
}
