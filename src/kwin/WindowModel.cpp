#include "WindowModel.h"
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/plasmawindowmanagement.h>
#include <QGuiApplication>
#include <KWayland/Client/plasmavirtualdesktop.h>
using namespace KWayland::Client;
WindowModel::WindowModel(QObject* parent):QObject(parent){
 if(QGuiApplication::platformName()!=QStringLiteral("wayland"))return;
 auto connection=ConnectionThread::fromApplication(this);if(!connection)return;
 registry=new Registry(this);
 connect(registry,&Registry::plasmaWindowManagementAnnounced,this,[this](quint32 name,quint32 version){
  manager=registry->createPlasmaWindowManagement(name,version,this);
  connect(manager,&PlasmaWindowManagement::activeWindowChanged,this,&WindowModel::changed);
  connect(manager,&PlasmaWindowManagement::windowCreated,this,[this](PlasmaWindow* w){
   connect(w,&PlasmaWindow::applicationMenuChanged,this,&WindowModel::changed);
   connect(w,&PlasmaWindow::titleChanged,this,&WindowModel::changed);
   connect(w,&PlasmaWindow::appIdChanged,this,&WindowModel::changed);
   connect(w,&PlasmaWindow::activeChanged,this,&WindowModel::changed);
   connect(w,&PlasmaWindow::unmapped,this,[this]{QMetaObject::invokeMethod(this,[this]{emit changed();},Qt::QueuedConnection);});emit changed();
  });emit changed();
 });
 connect(registry,&Registry::plasmaVirtualDesktopManagementAnnounced,this,[this](quint32 name,quint32 version){
  desktopManager=registry->createPlasmaVirtualDesktopManagement(name,version,this);
  connect(desktopManager,&PlasmaVirtualDesktopManagement::desktopCreated,this,[this](QString id,quint32){auto d=desktopManager->getVirtualDesktop(id);connect(d,&PlasmaVirtualDesktop::done,this,&WindowModel::changed);connect(d,&PlasmaVirtualDesktop::activated,this,&WindowModel::changed);connect(d,&PlasmaVirtualDesktop::deactivated,this,&WindowModel::changed);emit changed();});
  connect(desktopManager,&PlasmaVirtualDesktopManagement::desktopRemoved,this,&WindowModel::changed);
 });
 registry->create(connection);registry->setup();
}
QVariantList WindowModel::windows()const{QVariantList result;if(manager)for(auto w:manager->windows())if(!w->skipTaskbar())result.append(QVariantMap{{"id",QString::fromUtf8(w->uuid())},{"title",w->title()},{"appId",w->appId()},{"active",w->isActive()}});return result;}
QString WindowModel::activeTitle()const{auto w=manager?manager->activeWindow():nullptr;return w?w->title():QStringLiteral("Harbor");}
void WindowModel::activate(QString id){if(manager)for(auto w:manager->windows())if(w->uuid()==id.toUtf8())w->requestActivate();}
void WindowModel::close(QString id){if(manager)for(auto w:manager->windows())if(w->uuid()==id.toUtf8())w->requestClose();}
void WindowModel::minimize(QString id){if(manager)for(auto w:manager->windows())if(w->uuid()==id.toUtf8())w->requestToggleMinimized();}

QString WindowModel::menuService()const{auto w=manager?manager->activeWindow():nullptr;return w?w->applicationMenuServiceName():QString();}
QString WindowModel::menuPath()const{auto w=manager?manager->activeWindow():nullptr;return w?w->applicationMenuObjectPath():QString();}

QVariantList WindowModel::desktops()const{QVariantList out;if(desktopManager)for(auto d:desktopManager->desktops())out.append(QVariantMap{{"id",d->id()},{"name",d->name()},{"active",d->isActive()}});return out;}
void WindowModel::activateDesktop(QString id){if(desktopManager)for(auto d:desktopManager->desktops())if(d->id()==id)d->requestActivate();}
void WindowModel::addDesktop(){if(desktopManager&&desktopManager->desktops().size()<12)desktopManager->requestCreateVirtualDesktop(tr("Workspace ")+QString::number(desktopManager->desktops().size()+1));}
