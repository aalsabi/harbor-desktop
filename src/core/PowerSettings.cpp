#include "PowerSettings.h"
#include <QStandardPaths>
#include <QFile>
#include <QSaveFile>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QDateTime>
#include <QRegularExpression>
namespace {QString read(const QString &path){QFile f(path);if(!f.open(QIODevice::ReadOnly))return {};return QString::fromUtf8(f.readAll()).trimmed();}}
PowerSettings::PowerSettings(QObject *parent,QString configPath,QString sysfsRoot):QObject(parent),m_configPath(configPath.isEmpty()?QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/harbor/power.json":configPath),m_sysfsRoot(sysfsRoot.isEmpty()?"/sys/class/power_supply":sysfsRoot){
 m_timer.setInterval(2000);connect(&m_timer,&QTimer::timeout,this,&PowerSettings::refresh);
 connect(&m_command,&Command::finished,this,[this](bool ok,QString out){m_busy=false;if(!ok)m_error=out;else{auto result=QJsonDocument::fromJson(out.toUtf8()).object();m_error=result.contains("end")?tr("Charging stops at %1%. Values reflect the limits supported by the battery.").arg(result["end"].toInt()):tr("The helper returned an invalid charging result.");}refresh();});
}
QVariantList PowerSettings::readBatteries(const QString &root){QVariantList list;for(const auto &name:QDir(root).entryList(QDir::Dirs|QDir::NoDotAndDotDot)){QString path=root+"/"+name;if(read(path+"/type")!="Battery")continue;bool startOk=false,endOk=false;int start=read(path+"/charge_control_start_threshold").toInt(&startOk),end=read(path+"/charge_control_end_threshold").toInt(&endOk);list<<QVariantMap{{"name",name},{"status",read(path+"/status")},{"capacity",read(path+"/capacity")},{"canLimit",endOk&&end>=0&&end<=100},{"canStart",startOk&&start>=0&&start<=100},{"start",startOk?start:-1},{"end",endOk?end:100}};}return list;}
void PowerSettings::setActive(bool active){if(active){refresh();m_timer.start();}else m_timer.stop();}
void PowerSettings::refresh(){if(m_busy)return;auto settings=QJsonDocument::fromJson(read(m_configPath).toUtf8()).object();int display=qBound(0,settings["displayMinutes"].toInt(),240),suspend=qBound(0,settings["suspendMinutes"].toInt(),240);if(display!=m_display||suspend!=m_suspend){m_display=display;m_suspend=suspend;emit idlePreferencesChanged();}auto batteries=readBatteries(m_sysfsRoot);if(batteries!=m_batteries){m_batteries=batteries;emit batteriesChanged();}
 auto key=QString::fromLatin1(QCryptographicHash::hash(qgetenv("WAYLAND_DISPLAY"),QCryptographicHash::Sha256).toHex().left(12));auto path=QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation)+"/harbor/power-agent-"+key+".json";auto agent=QJsonDocument::fromJson(read(path).toUtf8()).object();double age=QDateTime::currentSecsSinceEpoch()-agent["time"].toDouble();bool alive=!agent.isEmpty()&&age>=-2&&age<6;
 m_idleAvailable=alive&&agent["available"].toBool()&&!qEnvironmentVariableIsSet("HARBOR_PRIVATE_BUS");
 if(!alive)m_idleStatus=tr("The Harbor idle service is not running. Sign in to a Harbor session to configure automatic display sleep and suspend.");
 else if(!agent["error"].toString().isEmpty())m_idleStatus=agent["error"].toString();
 else if(!agent["available"].toBool())m_idleStatus=tr("Install swayidle, kscreen and systemd to enable idle actions.");
 else if(agent["applied"].toObject()["displayMinutes"].toInt()!=m_display||agent["applied"].toObject()["suspendMinutes"].toInt()!=m_suspend)m_idleStatus=tr("Applying idle preferences…");
 else m_idleStatus=tr("Idle preferences are active for this Harbor session.");emit changed();
}
void PowerSettings::saveIdle(int display,int suspend){if(m_busy)return;if(!m_idleAvailable){m_error=tr("The Harbor idle service is unavailable in this session.");emit changed();return;}if(display<0||display>240||suspend<0||suspend>240||(display&&suspend&&suspend<display)){m_error=tr("Choose 0–240 minutes; suspend must follow display sleep.");emit changed();return;}QDir().mkpath(QFileInfo(m_configPath).absolutePath());QSaveFile f(m_configPath);if(!f.open(QIODevice::WriteOnly)){m_error=f.errorString();emit changed();return;}f.write(QJsonDocument(QJsonObject{{"displayMinutes",display},{"suspendMinutes",suspend}}).toJson());if(!f.commit()){m_error=f.errorString();emit changed();return;}m_error.clear();refresh();}
void PowerSettings::setChargeLimits(QString battery,int start,int end){if(m_busy)return;bool found=false;for(auto v:m_batteries){auto b=v.toMap();if(b["name"].toString()==battery&&b["canLimit"].toBool()&&(b["canStart"].toBool()?start>=0:start==-1))found=true;}if(!found||end<1||end>100||start>=end||start< -1||!QRegularExpression("^[A-Za-z0-9_][A-Za-z0-9_.-]{0,127}$").match(battery).hasMatch()){m_error=tr("Choose supported charging limits with start below stop.");emit changed();return;}m_busy=true;m_error.clear();emit changed();m_command.run("pkexec",{"/usr/libexec/harbor-charge-limit",battery,QString::number(start),QString::number(end)},120000);}
