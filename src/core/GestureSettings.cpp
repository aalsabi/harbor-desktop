#include "GestureSettings.h"
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSaveFile>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDir>
#include <QLockFile>
#include <QUuid>
namespace {
const QString plugin="harbor-custom-gestures";
QDBusMessage request(const QString &method,QVariantList args){auto msg=QDBusMessage::createMethodCall("org.kde.KWin","/Scripting","org.kde.kwin.Scripting",method);msg.setArguments(args);return msg;}
QString quoted(const QString &s){auto json=QString::fromUtf8(QJsonDocument(QJsonArray{s}).toJson(QJsonDocument::Compact));return json.mid(1,json.size()-2);}
}
GestureSettings::GestureSettings(QObject *parent,QDBusConnection bus,QString directory):QObject(parent),m_bus(bus),m_directory(directory.isEmpty()?QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/harbor/gestures":directory){
 m_path="/org/harbor/Gestures/g"+QUuid::createUuid().toString(QUuid::Id128);m_bus.registerObject(m_path,this,QDBusConnection::ExportAllSlots);m_timeout.setSingleShot(true);m_timeout.setInterval(5000);
 connect(&m_timeout,&QTimer::timeout,this,[this]{unloadAfterFailure();fail(tr("KWin could not initialize the gesture handlers. Custom gestures remain disabled."));});
 auto watcher=new QDBusServiceWatcher("org.kde.KWin",m_bus,QDBusServiceWatcher::WatchForOwnerChange,this);connect(watcher,&QDBusServiceWatcher::serviceOwnerChanged,this,[this](QString,QString,QString owner){m_active=false;if(m_busy)fail(tr("The display service restarted while applying gestures."));if(!owner.isEmpty()&&m_autoRestore)QTimer::singleShot(500,this,&GestureSettings::restore);else emit changed();});
 readConfig();
}
GestureSettings::~GestureSettings(){m_bus.unregisterObject(m_path);}
QStringList GestureSettings::keys(){return {"3-Up","3-Down","3-Left","3-Right","4-Up","4-Down","4-Left","4-Right"};}
bool GestureSettings::validMappings(const QVariantMap &mappings){const QStringList allowed{"none","launcher","control","notifications","settings","windows"};for(auto i=mappings.begin();i!=mappings.end();++i)if(!keys().contains(i.key())||i.value().metaType().id()!=QMetaType::QString||!allowed.contains(i.value().toString()))return false;return true;}
QString GestureSettings::script(const QVariantMap &mappings,const QString &service,const QString &path,const QString &token){
 if(!validMappings(mappings))return {};
 QString out="import QtQuick\nimport org.kde.kwin 3.0\nItem {\n";
 out+=" DBusCall { id: ready; service: "+quoted(service)+"; path: "+quoted(path)+"; dbusInterface: \"org.harbor.Gestures\"; method: \"Ready\"; arguments: ["+quoted(token)+"] }\n Component.onCompleted: ready.call()\n";
 int index=0;for(const auto &key:keys()){QString action=mappings.value(key,"none").toString();if(action=="none")continue;QString id="action"+QString::number(index++);out+=" DBusCall { id: "+id+"; service: \"org.harbor.Shell\"; path: \"/Shell\"; dbusInterface: \"org.harbor.Shell\"; method: \"Show\"; arguments: ["+quoted(action)+"] }\n";out+=" SwipeGestureHandler { fingerCount: "+key.left(1)+"; direction: SwipeGestureHandler.Direction."+key.mid(2)+"; deviceType: SwipeGestureHandler.Device.Touchpad; onActivated: "+id+".call() }\n";}return out+"}\n";
}
void GestureSettings::readConfig(){QFile f(m_directory+"/settings.json");QJsonObject o;if(f.open(QIODevice::ReadOnly))o=QJsonDocument::fromJson(f.readAll()).object();auto mappings=o["mappings"].toObject().toVariantMap();m_mappings.clear();for(const auto &key:keys())m_mappings[key]="none";if(validMappings(mappings))for(auto i=mappings.begin();i!=mappings.end();++i)m_mappings[i.key()]=i.value();m_enabled=o["enabled"].toBool()&&validMappings(mappings);}
void GestureSettings::refresh(){if(m_busy)return;readConfig();m_busy=true;const int generation=++m_generation;emit changed();auto w=new QDBusPendingCallWatcher(m_bus.asyncCall(request("isScriptLoaded",{plugin}),3000),this);connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,generation]{QDBusPendingReply<bool> r=*w;w->deleteLater();if(generation!=m_generation)return;m_busy=false;m_available=!r.isError();m_active=!r.isError()&&r.value();if(r.isError())m_error=r.error().message();else m_error.clear();emit changed();});}
void GestureSettings::restore(){m_autoRestore=true;if(m_busy)return;readConfig();if(m_enabled)apply(m_mappings,true);}
void GestureSettings::apply(const QVariantMap &mappings,bool enabled){
 if(m_busy)return;if(!validMappings(mappings)){fail(tr("Choose one of the supported gesture actions."));return;}
 bool assigned=false;for(auto v:mappings)if(v.toString()!="none")assigned=true;if(enabled&&!assigned){fail(tr("Assign an action before enabling custom gestures."));return;}
 QDir().mkpath(m_directory);m_lock=std::make_unique<QLockFile>(m_directory+"/apply.lock");if(!m_lock->tryLock()){fail(tr("Another Harbor window is updating gestures."));return;}
 m_draft=mappings;m_targetEnabled=enabled;m_busy=true;const int generation=++m_generation;m_error.clear();emit changed();
 auto w=new QDBusPendingCallWatcher(m_bus.asyncCall(request("unloadScript",{plugin}),3000),this);connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,generation]{QDBusPendingReply<bool> r=*w;w->deleteLater();if(generation!=m_generation)return;if(r.isError()){m_available=false;fail(r.error().message());return;}m_available=true;m_active=false;if(!m_targetEnabled){success();return;}QTimer::singleShot(0,this,&GestureSettings::load);});
}
void GestureSettings::load(){
 if(!m_busy)return;const int generation=m_generation;
 m_token=QUuid::createUuid().toString(QUuid::WithoutBraces);const QString generationDirectory=m_directory+"/generated-"+m_token;if(!QDir().mkdir(generationDirectory)){fail(tr("Could not create the gesture script directory."));return;}m_scriptPath=generationDirectory+"/main.qml";QSaveFile file(m_scriptPath);auto content=script(m_draft,m_bus.baseService(),m_path,m_token).toUtf8();if(!file.open(QIODevice::WriteOnly)||file.write(content)!=content.size()||!file.commit()){fail(tr("Could not write the gesture configuration."));return;}
 auto w=new QDBusPendingCallWatcher(m_bus.asyncCall(request("loadDeclarativeScript",{m_scriptPath,plugin}),3000),this);connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,generation]{QDBusPendingReply<int> r=*w;w->deleteLater();if(generation!=m_generation)return;if(r.isError()||r.value()<0){fail(r.isError()?r.error().message():tr("The gesture script could not be loaded."));return;}
  auto msg=QDBusMessage::createMethodCall("org.kde.KWin","/Scripting/Script"+QString::number(r.value()),"org.kde.kwin.Script","run");m_timeout.start();auto run=new QDBusPendingCallWatcher(m_bus.asyncCall(msg,3000),this);connect(run,&QDBusPendingCallWatcher::finished,this,[this,run,generation]{QDBusPendingReply<> reply=*run;run->deleteLater();if(generation!=m_generation)return;if(reply.isError()&&m_busy){unloadAfterFailure();fail(reply.error().message());}});
 });
}
void GestureSettings::Ready(const QString &token){if(!m_busy||!m_timeout.isActive()||token!=m_token)return;m_active=true;success();}
void GestureSettings::success(){m_timeout.stop();m_token.clear();// A fresh directory avoids Qt's cached directory listing as well as its component cache.
 for(const auto &name:QDir(m_directory).entryList({"generated-*"},QDir::Dirs|QDir::NoDotAndDotDot)){
  const QString path=m_directory+"/"+name;
  if(!QRegularExpression("^generated-[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$").match(name).hasMatch()||QFileInfo(path).isSymLink())continue;
  if(path+"/main.qml"!=m_scriptPath||!m_targetEnabled){QFile::remove(path+"/main.qml");QDir().rmdir(path);}
 }m_enabled=m_targetEnabled;m_mappings=m_draft;QSaveFile file(m_directory+"/settings.json");auto content=QJsonDocument(QJsonObject{{"enabled",m_enabled},{"mappings",QJsonObject::fromVariantMap(m_mappings)}}).toJson();if(!file.open(QIODevice::WriteOnly)||file.write(content)!=content.size()||!file.commit())m_error=tr("Gestures changed for this session, but the startup preference could not be saved.");m_busy=false;m_lock.reset();emit changed();}
void GestureSettings::fail(QString message){++m_generation;m_timeout.stop();m_token.clear();m_busy=false;m_error=message;m_lock.reset();emit changed();}
void GestureSettings::unloadAfterFailure(){m_active=false;m_bus.asyncCall(request("unloadScript",{plugin}),3000);}
