#ifndef ROUTERRESSOURCE_H
#define ROUTERRESSOURCE_H

#include <QObject>
#include <restressourceinterface.h>
#include <engine_common.h>
namespace Router {
class Matrix;
}

/** REST and C++ class hierarchy
 *
 *  Panel Interface
 *  ===============
 *
 *  has all the info on the ports plus the summarized errors (alarms) of any
 *  physical device representation that is mapped into the panel
 *  both xpoint and routing views on the outputs controlled are available
 *  xpoint will report -1 (invalid port) for destinations that are routed to a
 *  source not available on the panel
 *  routing lists the global UIDs of sources mapped to destinations on this
 *  panel, even if the source is not available on this panel view.
 *  implements the xpoint-set endpoint for easy numeric routing MAY offers a
 *  pass through route/(*) endpoint that allows setting routes across the complete system
 *
 *  Router Interface
 *  ================
 *  implements the panel interface for a specific router (or level on it), usually exposing all
 *  destinations and sources.
 *  additionally adds info on the specific router and its connection status to the system
 *
 *
 *  */
class PanelInterface : public QObject, public REST::SplitMethodRessourceInterface
{
    Q_OBJECT
public:
    explicit PanelInterface(REST::Controller * restController, QObject *parent = nullptr);

signals:

    // RessourceInterface interface
public:
    bool handleRequest(QString url, REST::HttpContext &context, QByteArray requestBody);
    static QJsonObject JsonForPort(Router::Matrix *mtx, Router::Port::Direction dir, int idx);

    // SplitMethodRessourceInterface interface
public:
    bool handleGet(QString url, REST::HttpContext &context);
    bool handlePut(QString url, REST::HttpContext &context, QByteArray data) override;
    bool handlePost(QString url, REST::HttpContext &context, QByteArray data) override;
    bool handleDelete(QString url, REST::HttpContext &context, QByteArray data) override;
    bool handlePatch(QString url, REST::HttpContext &context, QByteArray data) override;

protected:
    QJsonObject routerStatus(QString id);
};

#endif // ROUTERRESSOURCE_H
