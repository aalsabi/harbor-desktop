#include "Applications.h"
#include <QIcon>
#include <QStandardPaths>
#include <algorithm>
#include <QProcess>
#include <QPointer>
#include <QFileInfo>
#undef signals
#include <gio/gdesktopappinfo.h>
Applications::Applications(QObject* p):QObject(p){refresh();}
namespace {
QString iconName(GAppInfo* app) {
 auto icon=g_app_info_get_icon(app);
 if(icon && G_IS_FILE_ICON(icon)) {
  auto file=g_file_icon_get_file(G_FILE_ICON(icon));
  auto path=g_file_get_path(file);
  QString result=QString::fromUtf8(path);g_free(path);return result;
 }
 if(icon && G_IS_THEMED_ICON(icon)) {
  const char*const* names=g_themed_icon_get_names(G_THEMED_ICON(icon));
  if(names && names[0])return QString::fromUtf8(names[0]);
 }
 return {};
}
QString appKey(QString id) {
 if(id.endsWith(".desktop",Qt::CaseInsensitive))id.chop(8);
 return id.toCaseFolded();
}
}
void Applications::refresh(){
 list.clear();appIcons.clear();desktopAliases.clear();executables.clear();
 GList* apps=g_app_info_get_all();
 for(auto l=apps;l;l=l->next){
  auto a=G_APP_INFO(l->data);const char* rawId=g_app_info_get_id(a);if(!rawId)continue;
  const QString id=QString::fromUtf8(rawId),icon=iconName(a);
  desktopAliases.insert(appKey(id),appKey(id));
  if(!G_IS_DESKTOP_APP_INFO(a)||(!g_desktop_app_info_get_nodisplay(G_DESKTOP_APP_INFO(a))&&!g_desktop_app_info_get_is_hidden(G_DESKTOP_APP_INFO(a))))
   executables.insert(id,QString::fromUtf8(g_app_info_get_executable(a)));
  if(G_IS_DESKTOP_APP_INFO(a)){const char* wm=g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO(a));if(wm&&*wm)desktopAliases.insert(appKey(QString::fromUtf8(wm)),appKey(id));}
  if(!icon.isEmpty()){
   appIcons.insert(appKey(id),icon);
   if(G_IS_DESKTOP_APP_INFO(a)){
    const char* wmClass=g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO(a));
    if(wmClass && *wmClass)appIcons.insert(appKey(QString::fromUtf8(wmClass)),icon);
   }
  }
  if(!g_app_info_should_show(a))continue;
  list.append(QVariantMap{{"id",id},{"name",QString::fromUtf8(g_app_info_get_display_name(a))},{"icon",icon}});
 }
 g_list_free_full(apps,g_object_unref);
 std::sort(list.begin(),list.end(),[](auto a,auto b){return QString::localeAwareCompare(a.toMap()["name"].toString(),b.toMap()["name"].toString())<0;});
 emit changed();
}
QString Applications::iconForAppId(QString appId) const {
 return appIcons.value(appKey(appId),appId.isEmpty()?QStringLiteral("app"):appId);
}

QString Applications::desktopKey(QString id)const{return desktopAliases.value(appKey(id),appKey(id));}
void Applications::clearPending(QString id){if(auto timer=pending.take(id)){timer->stop();timer->deleteLater();emit changed();}}
void Applications::setWindows(QVariantList windows){
 runningWindows=windows;
 for(auto row:windows){const auto key=desktopKey(row.toMap()["appId"].toString());for(const auto& id:pending.keys())if(desktopKey(id)==key){clearPending(id);emit activateRequested(row.toMap()["id"].toString());}}
}
QString Applications::desktopForTool(QString tool)const{
 if(tool=="files")return "org.harbor.Files.desktop";
 if(tool!="terminal")return {};
 auto command=QFileInfo(QStandardPaths::findExecutable("x-terminal-emulator")).canonicalFilePath();
 auto name=QFileInfo(command).fileName();if(name.endsWith(".wrapper"))name.chop(8);
 auto candidates=executables.keys();candidates.sort();
 if(!name.isEmpty())for(const auto& id:candidates)if(QFileInfo(executables.value(id)).fileName()==name)return id;
 for(const auto& id:QStringList{"org.gnome.Console.desktop","org.kde.konsole.desktop","org.gnome.Terminal.desktop","xfce4-terminal.desktop","debian-xterm.desktop"})if(executables.contains(id))return id;
 return {};
}
bool Applications::launch(QString id){
 if(id.contains('/')||!id.endsWith(".desktop")){status=tr("Invalid application identifier");emit error(status);emit changed();return false;}
 for(auto row:runningWindows){auto window=row.toMap();if(desktopKey(window["appId"].toString())==desktopKey(id)){emit activateRequested(window["id"].toString());return true;}}
 if(pending.contains(id))return true;
 auto app=g_desktop_app_info_new(id.toUtf8().constData());if(!app){status=tr("Application is not installed: ")+id;emit error(status);emit changed();return false;}
 const auto filename=QString::fromUtf8(g_desktop_app_info_get_filename(app));g_object_unref(app);
 if(filename.isEmpty()){status=tr("Application has no desktop file: ")+id;emit error(status);emit changed();return false;}
 auto process=new QProcess(this);
 // GUI children can outlive gio; do not give them short-lived capture pipes.
 process->setProcessChannelMode(QProcess::ForwardedChannels);
 auto timer=new QTimer(this);timer->setSingleShot(true);pending.insert(id,timer);status.clear();emit changed();
 const QPointer<QTimer> ticket(timer);const QPointer<QProcess> child(process);
 connect(timer,&QTimer::timeout,this,[this,id,child]{clearPending(id);if(child&&child->state()!=QProcess::NotRunning)child->kill();status=tr("No application window appeared. You can try launching again.");emit error(status);emit changed();});
 connect(process,&QProcess::errorOccurred,this,[this,id,ticket,process](QProcess::ProcessError errorCode){if(errorCode!=QProcess::FailedToStart)return;process->deleteLater();if(!ticket||pending.value(id)!=ticket.data())return;clearPending(id);status=tr("Could not start application: ")+process->errorString();emit error(status);emit changed();});
 connect(process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this,id,ticket,process](int code,QProcess::ExitStatus exit){process->deleteLater();if(!ticket||pending.value(id)!=ticket.data())return;if(code!=0||exit!=QProcess::NormalExit){clearPending(id);status=tr("Could not start application: ")+id;emit error(status);emit changed();}});
 // Keep desktop-entry expansion and D-Bus activation in GIO, outside the UI thread.
 process->start("gio",{"launch",filename});timer->start(30000);return true;
}
