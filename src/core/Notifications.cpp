#include "Notifications.h"
#include <QDBusConnection>
#include <QTimer>
Notifications::Notifications(bool serve,QObject* p,QString path):QObject(p),policy(path){connect(&policy,&NotificationPreferences::changed,this,[this]{for(int i=rows.size()-1;i>=0;--i){auto row=rows[i].toMap();auto key=row.value("policyId").toString();if(!policy.enabled(key)){remove(row["id"].toUInt(),2);continue;}if(!policy.preview(key)){row["body"]=QString();row["summary"]=tr("Notification");rows[i]=row;}}emit changed();});if(serve){auto bus=QDBusConnection::sessionBus();registered=bus.registerService("org.freedesktop.Notifications");if(registered)bus.registerObject("/org/freedesktop/Notifications",this,QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllSignals);}}
uint Notifications::Notify(QString app,uint replaces,QString icon,QString summary,QString body,QStringList actions,QVariantMap hints,int expire){
 QString key=hints.value("desktop-entry",app).toString().left(256);if(key.isEmpty())key="unknown";policy.recordApplication(key,app);
 uint id=0;for(auto row:rows)if(row.toMap()["id"].toUInt()==replaces)id=replaces;
 if(!id)id=nextId++;
 for(int i=rows.size()-1;i>=0;--i)if(rows[i].toMap()["id"].toUInt()==id)rows.removeAt(i);
 if(!policy.enabled(key)){QTimer::singleShot(0,this,[this,id]{emit NotificationClosed(id,2);});emit changed();return id;}
 if(!policy.preview(key)){summary=tr("Notification");body.clear();}
 rows.prepend(QVariantMap{{"policyId",key},{"id",id},{"app",app.left(256)},{"summary",summary.left(1024)},{"body",body.left(8192)},{"icon",icon},{"actions",actions.mid(0,20)}});
 while(rows.size()>100){uint old=rows.last().toMap()["id"].toUInt();remove(old,1);}
 auto gen=++generations[id];if(expire!=0)QTimer::singleShot(expire<0?7000:qBound(1,expire,3600000),this,[this,id,gen]{if(generations.value(id)==gen)remove(id,1);});
 emit changed();return id;
}
void Notifications::CloseNotification(uint id){remove(id,3);}
void Notifications::remove(uint id,uint reason){for(int i=0;i<rows.size();++i)if(rows[i].toMap()["id"].toUInt()==id){rows.removeAt(i);generations.remove(id);emit NotificationClosed(id,reason);emit changed();return;}}
void Notifications::invoke(uint id,QString key){for(auto row:rows){auto m=row.toMap();if(m["id"].toUInt()!=id)continue;auto a=m["actions"].toStringList();for(int i=0;i+1<a.size();i+=2)if(a[i]==key){emit ActionInvoked(id,key);remove(id,2);return;}}}
