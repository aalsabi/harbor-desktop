#include "AccessibilitySettings.h"
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusMessage>
#include <QTimer>
AccessibilitySettings::AccessibilitySettings(QObject*p,QString path,QDBusConnection connection):QObject(p),bus(connection),settings(path.isEmpty()?QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/harbor/accessibility.ini":path,QSettings::IniFormat){auto dir=QFileInfo(settings.fileName()).absolutePath();QDir().mkpath(dir);watcher.addPath(dir);connect(&watcher,&QFileSystemWatcher::directoryChanged,this,[this]{settings.sync();loadReaderStatus();emit changed();});loadReaderStatus();}
void AccessibilitySettings::loadReaderStatus(){values["orcaAvailable"]=!QStandardPaths::findExecutable("orca").isEmpty();values["screenReaderEnabled"]=settings.value("screenReaderEnabled",false).toBool();QFile file(QFileInfo(settings.fileName()).absolutePath()+"/accessibility-status.json");if(file.open(QIODevice::ReadOnly)){auto state=QJsonDocument::fromJson(file.readAll()).object();values["readerStatus"]=state.value("status").toString();}else values["readerStatus"]=tr("Screen reader starts in your Harbor session.");}
void AccessibilitySettings::queryEffects(std::function<void(bool)> done){
 auto m=QDBusMessage::createMethodCall("org.kde.KWin","/Effects","org.freedesktop.DBus.Properties","GetAll");m<<QString("org.kde.kwin.Effects");auto w=new QDBusPendingCallWatcher(bus.asyncCall(m,4000),this);
 connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,done]{QDBusPendingReply<QVariantMap> reply=*w;w->deleteLater();values["effectsAvailable"]=!reply.isError();if(reply.isError()){problem=reply.error().message();values["loaded"]=QStringList{};values["active"]=QStringList{};values["available"]=QStringList{};}else{auto props=reply.value();values["loaded"]=props.value("loadedEffects");values["available"]=props.value("listOfEffects");values["active"]=props.value("activeEffects");}loadReaderStatus();emit changed();done(!reply.isError());});
}
void AccessibilitySettings::finish(QString error){working=false;problem=error;emit changed();if(restoring){if(!error.isEmpty())restoreErrors.append(error);QTimer::singleShot(0,this,&AccessibilitySettings::restoreNext);}}
void AccessibilitySettings::refresh(){if(working)return;working=true;problem.clear();emit changed();queryEffects([this](bool ok){finish(ok?QString():problem);});}
void AccessibilitySettings::setEffect(QString name,bool enabled){
 if(working||!QStringList{"zoom","invert","trackmouse","mouseclick"}.contains(name))return;working=true;problem.clear();emit changed();
 queryEffects([this,name,enabled](bool ok){if(!ok){finish(problem);return;}if(!values.value("available").toStringList().contains(name)){finish(tr("This visual aid is not available from the compositor."));return;}applyEffect(name,enabled);});
}
void AccessibilitySettings::applyEffect(QString name,bool enabled){
 bool loaded=values.value("loaded").toStringList().contains(name);
 auto activate=[this,name,enabled]{
  if(name!="invert"||!enabled||values.value("active").toStringList().contains(name)){verifyEffect(name,enabled,3);return;}
  auto msg=QDBusMessage::createMethodCall("org.kde.kglobalaccel","/component/kwin","org.kde.kglobalaccel.Component","invokeShortcut");msg<<QString("Invert");auto w=new QDBusPendingCallWatcher(bus.asyncCall(msg,4000),this);
  connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,name,enabled]{auto reply=w->reply();w->deleteLater();bool denied=!reply.arguments().isEmpty()&&reply.arguments().first().metaType().id()==QMetaType::Bool&&!reply.arguments().first().toBool();if(reply.type()==QDBusMessage::ErrorMessage||denied){QString error=reply.type()==QDBusMessage::ErrorMessage?reply.errorMessage():tr("The compositor did not activate color inversion.");queryEffects([this,error](bool){finish(error);});return;}QTimer::singleShot(80,this,[this,name,enabled]{verifyEffect(name,enabled,3);});});
 };
 if(loaded==enabled){activate();return;}
 auto msg=QDBusMessage::createMethodCall("org.kde.KWin","/Effects","org.kde.kwin.Effects",enabled?"loadEffect":"unloadEffect");msg<<name;auto w=new QDBusPendingCallWatcher(bus.asyncCall(msg,5000),this);
 connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,activate]{auto reply=w->reply();w->deleteLater();if(reply.type()==QDBusMessage::ErrorMessage||(!reply.arguments().isEmpty()&&reply.arguments().first().metaType().id()==QMetaType::Bool&&!reply.arguments().first().toBool())){finish(reply.type()==QDBusMessage::ErrorMessage?reply.errorMessage():tr("The compositor could not enable this visual aid."));return;}queryEffects([this,activate](bool ok){if(!ok){finish(problem);return;}activate();});});
}
void AccessibilitySettings::verifyEffect(QString name,bool enabled,int attempts){
 queryEffects([this,name,enabled,attempts](bool ok){if(!ok){finish(problem);return;}bool actual=values.value(name=="invert"?"active":"loaded").toStringList().contains(name);if(actual!=enabled){if(attempts>1){QTimer::singleShot(100,this,[this,name,enabled,attempts]{verifyEffect(name,enabled,attempts-1);});return;}finish(tr("The compositor did not apply the requested visual aid state."));return;}settings.setValue("effects/"+name,enabled);settings.sync();finish(settings.status()==QSettings::NoError?QString():tr("The visual aid changed, but its preference could not be saved."));});
}
void AccessibilitySettings::zoom(QString action){static const QMap<QString,QString> actions{{"in","Zoom In"},{"out","Zoom Out"},{"reset","Actual Size"}};if(working||!actions.contains(action))return;working=true;problem.clear();emit changed();auto m=QDBusMessage::createMethodCall("org.kde.kglobalaccel","/component/kwin","org.kde.kglobalaccel.Component","invokeShortcut");m<<actions[action];auto w=new QDBusPendingCallWatcher(bus.asyncCall(m,4000),this);connect(w,&QDBusPendingCallWatcher::finished,this,[this,w]{auto r=w->reply();w->deleteLater();QString error=r.type()==QDBusMessage::ErrorMessage?r.errorMessage():QString();if(error.isEmpty()&&!r.arguments().isEmpty()&&r.arguments().first().metaType().id()==QMetaType::Bool&&!r.arguments().first().toBool())error=tr("The compositor did not apply the zoom shortcut.");queryEffects([this,error](bool ok){finish(!error.isEmpty()?error:ok?QString():problem);});});}
void AccessibilitySettings::setScreenReader(bool enabled){if(enabled&&!values.value("orcaAvailable").toBool()){problem=tr("Install Orca to use the screen reader.");emit changed();return;}settings.setValue("screenReaderEnabled",enabled);settings.sync();problem=settings.status()==QSettings::NoError?QString():tr("Could not save the screen reader preference.");loadReaderStatus();emit changed();}
void AccessibilitySettings::restore(){if(working||restoring)return;settings.sync();restoreErrors.clear();restoreQueue.clear();for(const auto &name:QStringList{"zoom","invert","trackmouse","mouseclick"})if(settings.contains("effects/"+name))restoreQueue.append(name);restoring=true;restoreNext();}
void AccessibilitySettings::restoreNext(){if(restoreQueue.isEmpty()){restoring=false;problem=restoreErrors.join("\n");emit changed();return;}auto name=restoreQueue.takeFirst();setEffect(name,settings.value("effects/"+name).toBool());}
