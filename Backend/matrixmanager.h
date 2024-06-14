#ifndef ROUTER_MATRIXMANAGER_H
#define ROUTER_MATRIXMANAGER_H

#include <QObject>
#include "matrix.h"

namespace Router {

/**
* @brief The MatrixManager class
* The Matrix Manager takes care of loading and saving matrix configurations,
* creating panels for physical matrices in the frontend subsys
* and of registering the ports of each matrix to the router engine.
*
* menonics and port UIDs are handled in the Matrix class which can be seen as the Model
* in an MVC approach
*
*/

class MatrixManager : public QObject
{
    Q_OBJECT
public:
    enum {
        alarmMatrixOffline    = 0x0001,
        alarmCommsError       = 0x0100
    };

    static MatrixManager *inst();

    bool load(QJsonObject&);
    QJsonObject getConfig();
    int alarms() const;


private:

    explicit MatrixManager(QObject *parent = nullptr);
    static MatrixManager * mInst;

    QMap<QString, Matrix> mRouters;
    int mAlarmState;

    bool mDebug;

signals:
    void xPointChanged(
        QString busId,
        int addr, int level, int dst, int src);

public slots:

};

} // namespace Router

#endif // ROUTER_MATRIXMANAGER_H
