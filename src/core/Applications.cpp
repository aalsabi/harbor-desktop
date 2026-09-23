#include "Applications.h"
#include <QIcon>
#include <QStandardPaths>
#include <algorithm>
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
 list.clear();appIcons.clear();
 GList* apps=g_app_info_get_all();
 for(auto l=apps;l;l=l->next){
  auto a=G_APP_INFO(l->data);const char* rawId=g_app_info_get_id(a);if(!rawId)continue;
  const QString id=QString::fromUtf8(rawId),icon=iconName(a);
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
bool Applications::launch(QString id){if(id.contains('/')||!id.endsWith(".desktop")){emit error(tr("Invalid application identifier"));return false;}auto app=g_desktop_app_info_new(id.toUtf8().constData());if(!app){emit error(tr("Application is not installed: ")+id);return false;}GError* e=nullptr;bool ok=g_app_info_launch(G_APP_INFO(app),nullptr,nullptr,&e);if(e){emit error(QString::fromUtf8(e->message));g_error_free(e);}g_object_unref(app);return ok;}
