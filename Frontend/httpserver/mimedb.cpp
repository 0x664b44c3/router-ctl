#include "mimedb.h"
#include <QHash>

MimeDB::MimeDB(const QString defaultMIME) :
	m_Default(defaultMIME)
{

}

void MimeDB::addExtension(QString ext, QString mime)
{
	m_Types[ext] = mime;
}

QString MimeDB::mime(QString fileName) const
{
	int loc = fileName.lastIndexOf('.') + 1;
	if ((loc<0) || (loc>=fileName.length()))
		return m_Default;
	QString ext = fileName.mid(loc);
	return m_Types.value(ext, m_Default);
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
}
