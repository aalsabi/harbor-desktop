#include "GlobalMenu.h"
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDateTime>
#include <QTimer>
QDBusArgument& operator<<(QDBusArgument& a,const MenuLayout& v){a.beginStructure();a<<v.id<<v.properties<<v.children;a.endStructure();return a;}
const QDBusArgument& operator>>(const QDBusArgument& a,MenuLayout& v){a.beginStructure();a>>v.id>>v.properties>>v.children;a.endStructure();return a;}
GlobalMenu::GlobalMenu(QObject* p):QObject(p){qDBusRegisterMetaType<MenuLayout>();auto timer=new QTimer(this);timer->start(3000);connect(timer,&QTimer::timeout,this,&GlobalMenu::load);}
void GlobalMenu::setSource(QString s,QString p){if(service==s&&path==p)return;auto bus=QDBusConnection::sessionBus();if(!service.isEmpty())bus.disconnect(service,path,"com.canonical.dbusmenu","LayoutUpdated",this,SLOT(reload()));service=s;path=p;if(!service.isEmpty())bus.connect(service,path,"com.canonical.dbusmenu","LayoutUpdated",this,SLOT(reload()));selected=0;top.clear();current.clear();all.clear();++generation;emit changed();load();}
void GlobalMenu::load(){if(service.isEmpty()||path.isEmpty())return;auto msg=QDBusMessage::createMethodCall(service,path,"com.canonical.dbusmenu","GetLayout");msg<<0<<-1<<QStringList{};auto watch=new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(msg,2000),this);uint gen=generation;
 connect(watch,&QDBusPendingCallWatcher::finished,this,[this,watch,gen]{QDBusPendingReply<uint,MenuLayout> reply=*watch;watch->deleteLater();if(gen!=generation)return;all.clear();top.clear();if(!reply.isError()){flatten(reply.argumentAt<1>());for(auto v:all)if(v.toMap()["parent"].toInt()==0)top.append(v);}select(selected);emit changed();});}
void GlobalMenu::flatten(const MenuLayout& layout,int depth){if(depth>8||all.size()>300)return;for(auto child:layout.children){auto v=qdbus_cast<MenuLayout>(child.variant());auto m=v.properties;if(m.value("visible",true).toBool()&&m.value("type").toString()!="separator"){m["id"]=v.id;m["parent"]=layout.id;m["label"]=m.value("label").toString().remove('_');m["enabled"]=m.value("enabled",true);m["submenu"]=!v.children.isEmpty();all.append(m);flatten(v,depth+1);}}}
void GlobalMenu::select(int id){selected=id;current.clear();for(auto v:all)if(v.toMap()["parent"].toInt()==id)current.append(v);emit changed();}
void GlobalMenu::back(){for(auto v:all)if(v.toMap()["id"].toInt()==selected){select(v.toMap()["parent"].toInt());return;}select(0);}
void GlobalMenu::trigger(int id){for(auto v:all){auto m=v.toMap();if(m["id"].toInt()!=id||!m["enabled"].toBool())continue;if(m["submenu"].toBool()){auto msg=QDBusMessage::createMethodCall(service,path,"com.canonical.dbusmenu","AboutToShow");msg<<id;QDBusConnection::sessionBus().asyncCall(msg,2000);select(id);load();return;}auto msg=QDBusMessage::createMethodCall(service,path,"com.canonical.dbusmenu","Event");msg<<id<<QString("clicked")<<QVariant::fromValue(QDBusVariant(0))<<uint(QDateTime::currentMSecsSinceEpoch());QDBusConnection::sessionBus().asyncCall(msg,2000);emit activated();return;}}
