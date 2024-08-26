#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <QObject>

namespace Router {

class Snapshot : public QObject
{
    Q_OBJECT
public:
    explicit Snapshot(QObject *parent = nullptr);

signals:
};

} // namespace Router

#endif // SNAPSHOT_H
