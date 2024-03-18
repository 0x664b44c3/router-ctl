#ifndef ROUTER_MATRIX_H
#define ROUTER_MATRIX_H

#include <QObject>
#include "engine_common.h"
#include <QJsonObject>

namespace Router {

class Matrix : public QObject
{
    Q_OBJECT

public:

    struct PortInfo {
        /**
         * @brief id - a local ID for each port
         * this ID - together with the matrix' UID - is the default network-wide unique ID of a port
         * it is auto-initialized upon creation of a matrix object and will be re-generated based on
         * a fixced schema when loading a matrix config with missing ids
         * schema is in-XX for inputs and out-xx for outputs, XX being a decimal number
         * an unique ID is obtained by prepending the matrix' uid with a dash ('-') inbetween (e.g. matrix01-in-01)
         */
        QString id;
        QString uid; ///< user definable uid, will directly be visible system-wide
        QString label; ///< short text, usually <16 chars but can be longer
        QString mnemonic; ///< very short (4 or 8 chars) text to allow labelling of button panels
    }
    explicit Matrix(QObject *parent = nullptr);
    bool setBusAddress(QString bus, int level=0, int frame=0);

    std::vector<int> getLockoutList(int dest);
    bool setLockoutList(int dest, std::vector<int> sources);

    int level() const;
    void setLevel(int newLevel);

    int frameId() const;
    void setFrameId(int newFrameId);

    QString busId() const;
    void setBusId(const QString &newBusId);

    //config load/save
    bool loadConfig(const QJsonObject &);
    QJsonObject getConfig() const;

public slots:
    void setXPoint(int dest, int source);
    void setLabel(Port::Direction dir, int id, QString label);
    ///
    /// \brief lockPort - locks the port on the router (if available in the underlying control protocol)
    /// \param dest - numeric destination id
    ///
    void lockPort(int dest);
    /// @brief unlockPort - unlocks a port on the router
    void unlockPort(int dest);

signals:
    void xPointChanged(int dest, int source);
    void labelChanged(Port::Direction dir, int id, QString label);
    void portLocked(int dest, bool lock);

private:
    int mLevel;
    int mFrameId;
    QString mBusId;
    QStringList mInputIds;
    QStringList mOutputIds;
};

} // namespace Router

#endif // ROUTER_MATRIX_H
