#include "Applications.h"
#include <QIcon>
#include <QStandardPaths>
#include <algorithm>
#undef signals
#include <gio/gdesktopappinfo.h>
Applications::Applications(QObject* p):QObject(p){refresh();}
void Applications::refresh(){list.clear();GList* apps=g_app_info_get_all();for(auto l=apps;l;l=l->next){auto a=G_APP_INFO(l->data);if(!g_app_info_should_show(a))continue;const char* id=g_app_info_get_id(a);if(!id)continue;QString icon;auto i=g_app_info_get_icon(a);if(i&&G_IS_THEMED_ICON(i)){const char*const* names=g_themed_icon_get_names(G_THEMED_ICON(i));if(names&&names[0])icon=QString::fromUtf8(names[0]);}list.append(QVariantMap{{"id",QString::fromUtf8(id)},{"name",QString::fromUtf8(g_app_info_get_display_name(a))},{"icon",icon}});}g_list_free_full(apps,g_object_unref);std::sort(list.begin(),list.end(),[](auto a,auto b){return QString::localeAwareCompare(a.toMap()["name"].toString(),b.toMap()["name"].toString())<0;});emit changed();}
bool Applications::launch(QString id){if(id.contains('/')||!id.endsWith(".desktop")){emit error(tr("Invalid application identifier"));return false;}auto app=g_desktop_app_info_new(id.toUtf8().constData());if(!app){emit error(tr("Application is not installed: ")+id);return false;}GError* e=nullptr;bool ok=g_app_info_launch(G_APP_INFO(app),nullptr,nullptr,&e);if(e){emit error(QString::fromUtf8(e->message));g_error_free(e);}g_object_unref(app);return ok;}
