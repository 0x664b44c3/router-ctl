#ifndef MIMEDB_H
#define MIMEDB_H

#include <QHash>
#include <QString>

class MimeDB {
public:
	explicit MimeDB(const QString defaultMIME = "application/octet-stream");
	void addExtension(QString ext, QString mime);
	QString mime(QString fileName) const;
	void setupDefaultTypes();
private:
	QHash<QString, QString> m_Types;
	QString m_Default;
};

#endif // MIMEDB_H
