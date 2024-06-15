#ifndef MIMEDB_H
#define MIMEDB_H

#include <QHash>
#include <QString>

class MimeDB {
public:
	void addExtension(QString ext, QString mime);
	QString mime(QString fileName) const;
    void setDefaultMimeType(QString);
	void setupDefaultTypes();
    static MimeDB * inst();
    QString defaultMimeType() const;

private:
    explicit MimeDB(const QString defaultMIME = "application/octet-stream");
    static MimeDB *mInst;
    QHash<QString, QString> mTypes;
    QString mDefaultMimeType;
};

#endif // MIMEDB_H
