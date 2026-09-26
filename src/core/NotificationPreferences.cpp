#include "NotificationPreferences.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUuid>
#include <QSet>
NotificationPreferences::NotificationPreferences(QString path,QObject*parent):QObject(parent),settings(path.isEmpty()?QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/harbor/notifications.ini":path,QSettings::IniFormat){
 timer.setInterval(15000);connect(&timer,&QTimer::timeout,this,[this]{auto id=activeProfile().value("id").toString();if(id!=lastActive){lastActive=id;emit changed();}});timer.start();
 auto dir=QFileInfo(settings.fileName()).absolutePath();QDir().mkpath(dir);watcher.addPath(dir);
 connect(&watcher,&QFileSystemWatcher::directoryChanged,this,[this]{settings.sync();emit changed();});
}
QString NotificationPreferences::group(QString id)const{return "apps/"+QString::fromLatin1(QCryptographicHash::hash(id.toUtf8(),QCryptographicHash::Sha256).toHex())+"/";}
bool NotificationPreferences::doNotDisturb()const{return settings.value("doNotDisturb",false).toBool();}
bool NotificationPreferences::enabled(QString id)const{return settings.value(group(id)+"enabled",true).toBool();}
bool NotificationPreferences::preview(QString id)const{return settings.value(group(id)+"preview",true).toBool();}
QVariantList NotificationPreferences::applications()const{
 QVariantList rows;auto keys=settings.allKeys();for(auto key:keys)if(key.startsWith("apps/")&&key.endsWith("/id")){auto id=settings.value(key).toString();rows.append(QVariantMap{{"id",id},{"name",settings.value(group(id)+"name",id)},{"enabled",enabled(id)},{"preview",preview(id)}});}return rows;
}
void NotificationPreferences::recordApplication(QString id,QString name){
 if(id.isEmpty()||id.size()>256)return;
 auto prefix=group(id);if(settings.contains(prefix+"id"))return;if(applications().size()>=256)return;
 settings.setValue(prefix+"id",id);settings.setValue(prefix+"name",name.left(256));settings.sync();emit changed();
}
void NotificationPreferences::setDoNotDisturb(bool value){settings.setValue("doNotDisturb",value);settings.sync();emit changed();}
void NotificationPreferences::set(QString id,QString key,bool value){if(!settings.contains(group(id)+"id"))return;settings.setValue(group(id)+key,value);settings.sync();emit changed();}
void NotificationPreferences::setEnabled(QString id,bool value){set(id,"enabled",value);}
void NotificationPreferences::setPreview(QString id,bool value){set(id,"preview",value);}

QVariantList NotificationPreferences::profiles()const{return QJsonDocument::fromJson(settings.value("focus/profiles").toByteArray()).array().toVariantList();}
QString NotificationPreferences::focusMode()const{return settings.value("focus/mode","auto").toString();}
bool NotificationPreferences::scheduleMatches(const QVariantMap &p,const QDateTime &now){
 if(!p.value("scheduled").toBool()||!now.isValid())return false;
 auto start=QTime::fromString(p.value("start").toString(),"HH:mm"),end=QTime::fromString(p.value("end").toString(),"HH:mm");
 if(!start.isValid()||!end.isValid()||start==end)return false;
 int day=now.date().dayOfWeek();auto time=now.time();
 if(start>end){if(time<end)day=day==1?7:day-1;else if(time<start)return false;}else if(time<start||time>=end)return false;
 for(auto d:p.value("days").toList())if(d.toInt()==day)return true;return false;
}
QVariantMap NotificationPreferences::activeProfile()const{auto mode=focusMode();if(mode=="off")return {};for(auto value:profiles()){auto p=value.toMap();if((mode=="auto"&&scheduleMatches(p,QDateTime::currentDateTime()))||mode==p.value("id").toString())return p;}return {};}
bool NotificationPreferences::attentionAllowed(QString id)const{if(doNotDisturb())return false;auto active=activeProfile();return active.isEmpty()||active.value("allowed").toStringList().contains(id);}
void NotificationPreferences::setFocusMode(QString mode){settings.sync();bool known=mode=="auto"||mode=="off";for(auto p:profiles())known|=p.toMap().value("id").toString()==mode;if(!known)return;settings.setValue("focus/mode",mode);settings.sync();problem=settings.status()==QSettings::NoError?QString():tr("Could not save Focus mode.");emit changed();}
bool NotificationPreferences::saveProfile(QVariantMap p){
 settings.sync();
 auto fail=[this](QString message){problem=message;emit changed();return false;};auto name=p.value("name").toString().trimmed();auto start=QTime::fromString(p.value("start").toString(),"HH:mm"),end=QTime::fromString(p.value("end").toString(),"HH:mm");
 if(name.isEmpty()||name.size()>80||!start.isValid()||!end.isValid()||start==end)return fail(tr("Enter a name and different valid start/end times (HH:mm)."));
 auto days=p.value("days").toList();QSet<int> seen;for(auto d:days){bool ok=false;int day=d.toString().toInt(&ok);if(!ok||day<1||day>7||seen.contains(day))return fail(tr("Choose each weekday only once."));seen.insert(day);}if(p.value("scheduled").toBool()&&days.isEmpty())return fail(tr("Choose at least one scheduled day."));
 QStringList allowed=p.value("allowed").toStringList();if(allowed.size()>256)return fail(tr("Too many allowed applications."));for(auto id:allowed)if(id.isEmpty()||id.size()>256)return fail(tr("Invalid application identifier."));allowed.removeDuplicates();
 auto rows=profiles();auto id=p.value("id").toString();int index=-1;for(int i=0;i<rows.size();++i)if(rows[i].toMap().value("id").toString()==id)index=i;
 if(!id.isEmpty()&&index<0)return fail(tr("This Focus profile no longer exists."));if(index<0&&rows.size()>=16)return fail(tr("A maximum of 16 Focus profiles is supported."));if(id.isEmpty())id=QUuid::createUuid().toString(QUuid::WithoutBraces);
 QVariantMap clean{{"id",id},{"name",name},{"start",start.toString("HH:mm")},{"end",end.toString("HH:mm")},{"days",days},{"scheduled",p.value("scheduled").toBool()},{"allowed",allowed}};
 if(index<0)rows.append(clean);else rows[index]=clean;settings.setValue("focus/profiles",QJsonDocument(QJsonArray::fromVariantList(rows)).toJson(QJsonDocument::Compact));settings.sync();if(settings.status()!=QSettings::NoError)return fail(tr("Could not save Focus profile."));problem.clear();emit changed();return true;
}
void NotificationPreferences::removeProfile(QString id){settings.sync();auto rows=profiles();for(int i=rows.size()-1;i>=0;--i)if(rows[i].toMap().value("id").toString()==id)rows.removeAt(i);settings.setValue("focus/profiles",QJsonDocument(QJsonArray::fromVariantList(rows)).toJson(QJsonDocument::Compact));if(focusMode()==id)settings.setValue("focus/mode","auto");settings.sync();problem=settings.status()==QSettings::NoError?QString():tr("Could not remove Focus profile.");emit changed();}
