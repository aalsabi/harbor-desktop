#include "Accounts.h"
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusObjectPath>
#include <QRegularExpression>
void Accounts::refresh(){uint gen=++generation;auto msg=QDBusMessage::createMethodCall("org.freedesktop.Accounts","/org/freedesktop/Accounts","org.freedesktop.Accounts","ListCachedUsers");auto w=new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(msg,5000),this);
 connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,gen]{QDBusPendingReply<QList<QDBusObjectPath>> r=*w;w->deleteLater();if(gen!=generation)return;rows.clear();problem=r.isError()?r.error().message():QString();emit changed();if(r.isError())return;for(auto path:r.value()){
 auto m=QDBusMessage::createMethodCall("org.freedesktop.Accounts",path.path(),"org.freedesktop.DBus.Properties","GetAll");m<<QString("org.freedesktop.Accounts.User");auto q=new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(m,5000),this);
 connect(q,&QDBusPendingCallWatcher::finished,this,[this,q,path,gen]{QDBusPendingReply<QVariantMap> r=*q;q->deleteLater();if(gen!=generation||r.isError())return;auto m=r.value();m["path"]=path.path();rows.append(m);emit changed();});
 }});}
void Accounts::setRealName(QString path,QString name){if(working||name.trimmed().isEmpty()||name.size()>200)return;bool found=false;for(auto row:rows)if(row.toMap()["path"]==path)found=true;if(!found)return;working=true;problem.clear();emit changed();
 auto msg=QDBusMessage::createMethodCall("org.freedesktop.Accounts",path,"org.freedesktop.Accounts.User","SetRealName");msg<<name.trimmed();auto w=new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(msg,60000),this);
 connect(w,&QDBusPendingCallWatcher::finished,this,[this,w]{QDBusPendingReply<> r=*w;w->deleteLater();working=false;problem=r.isError()?r.error().message():QString();emit changed();if(!r.isError())refresh();});}
