#pragma once
#include <QObject>
#include <QVariantList>
namespace KWayland::Client {class Registry;class PlasmaWindowManagement;class PlasmaVirtualDesktopManagement;}
class WindowModel:public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantList desktops READ desktops NOTIFY changed)
 Q_PROPERTY(QString menuService READ menuService NOTIFY changed)
 Q_PROPERTY(QString menuPath READ menuPath NOTIFY changed)
 Q_PROPERTY(QVariantList windows READ windows NOTIFY changed)
 Q_PROPERTY(QString activeTitle READ activeTitle NOTIFY changed)
 Q_PROPERTY(bool available READ available NOTIFY changed)
public:explicit WindowModel(QObject* parent=nullptr);QVariantList windows()const;QString activeTitle()const;QString menuService()const;QString menuPath()const;bool available()const{return manager;}
 QVariantList desktops()const;
 Q_INVOKABLE void activateDesktop(QString id);Q_INVOKABLE void addDesktop();
 Q_INVOKABLE void activate(QString id);Q_INVOKABLE void close(QString id);Q_INVOKABLE void minimize(QString id);
signals:void changed();
private:KWayland::Client::PlasmaVirtualDesktopManagement* desktopManager=nullptr;KWayland::Client::Registry* registry=nullptr;KWayland::Client::PlasmaWindowManagement* manager=nullptr;
};
