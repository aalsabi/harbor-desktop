#include "DisplaySettings.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QUuid>
#include <cmath>
DisplaySettings::DisplaySettings(QObject *parent,int transactionTimeoutMs):QObject(parent),m_transactionTimeoutMs(qMax(1,transactionTimeoutMs)){connect(&m_command,&Command::finished,this,&DisplaySettings::complete);m_poll.setInterval(250);connect(&m_poll,&QTimer::timeout,this,&DisplaySettings::poll);}
void DisplaySettings::refresh(){if(m_busy)return;m_busy=true;m_stage=1;m_error.clear();emit changed();m_command.run("kscreen-doctor",{"-j"});}
QVariantList DisplaySettings::parseMonitors(const QString &out){
 QVariantList result;QVariantMap row;QRegularExpression display("^Display\\s+([0-9]+)"),bus("I2C bus:\\s*/dev/i2c-([0-9]+)");
 auto append=[&]{if(row.contains("bus")&&result.size()<16)result<<row;row.clear();};
 for(const auto &line:out.split('\n')){auto text=line.trimmed();if(text.startsWith("Invalid display")){append();continue;}if(display.match(text).hasMatch()){append();row["name"]=text;}auto match=bus.match(text);if(match.hasMatch()&&!row.isEmpty())row["bus"]=match.captured(1).toInt();if(text.startsWith("Monitor:")&&!row.isEmpty())row["name"]=text.mid(8).trimmed();}append();return result;
}
QVariantMap DisplaySettings::parseVcp(const QString &out){auto m=QRegularExpression("(?:^|\\n)VCP\\s+10\\s+C\\s+(\\d+)\\s+(\\d+)").match(out);if(!m.hasMatch())return {};int value=m.captured(1).toInt(),max=m.captured(2).toInt();if(max<=0||value>max)return {};return {{"value",value},{"maximum",max},{"percent",qRound(100.0*value/max)}};}
void DisplaySettings::detectBrightness(){if(m_busy)return;m_busy=true;m_stage=2;m_brightnessError.clear();m_monitors.clear();emit changed();m_command.run("ddcutil",{"detect","--brief"},20000);}
void DisplaySettings::nextMonitor(){if(m_index>=m_candidates.size()){m_busy=false;if(m_monitors.isEmpty()&&m_brightnessError.isEmpty())m_brightnessError=tr("No monitor exposing DDC/CI brightness was found. Check monitor DDC/CI settings and I²C permissions.");emit changed();return;}m_stage=3;m_command.run("ddcutil",{"--bus",QString::number(m_candidates[m_index].toMap()["bus"].toInt()),"getvcp","10","--terse"},8000);}
void DisplaySettings::complete(bool ok,QString out){
 if(m_stage==1){auto doc=QJsonDocument::fromJson(out.toUtf8());if(ok&&doc.isObject()&&doc.object()["outputs"].isArray()){m_outputs.clear();for(auto v:doc.object()["outputs"].toArray()){auto o=v.toObject();if(o["connected"].toBool()&&o["enabled"].toBool())m_outputs<<o.toVariantMap();}}else{m_outputs.clear();m_error=out.isEmpty()?tr("Display information is unavailable."):out;}m_busy=false;emit changed();return;}
 if(m_stage==2){if(!ok){m_brightnessError=out;m_busy=false;emit changed();return;}m_candidates=parseMonitors(out);m_index=0;nextMonitor();return;}
 if(m_stage==3){if(!ok&&m_brightnessError.isEmpty())m_brightnessError=tr("Could not read monitor brightness: %1").arg(out);auto value=ok?parseVcp(out):QVariantMap{};if(!value.isEmpty()){auto row=m_candidates[m_index].toMap();for(auto i=value.begin();i!=value.end();++i)row[i.key()]=i.value();m_monitors<<row;}++m_index;nextMonitor();return;}
 if(m_stage==4){m_busy=false;if(!ok){m_brightnessError=out;emit changed();return;}detectBrightness();}
}
void DisplaySettings::setBrightness(int bus,int percent){if(m_busy||percent<0||percent>100)return;QVariantMap target;for(auto m:m_monitors)if(m.toMap()["bus"].toInt()==bus)target=m.toMap();if(target.isEmpty())return;m_busy=true;m_stage=4;m_brightnessError.clear();emit changed();m_command.run("ddcutil",{"--bus",QString::number(bus),"setvcp","10",QString::number(qRound(percent*target["maximum"].toInt()/100.0)),"--verify"},10000);}
void DisplaySettings::apply(int id,int x,int y,double scale,QString mode,bool primary){
 if(m_busy||!std::isfinite(scale)||scale<.5||scale>4||x<0||y<0||x>32768||y>32768)return;
 bool valid=false;for(auto v:m_outputs){auto row=v.toMap();if(row["id"].toInt()==id)for(auto m:row["modes"].toList())if(m.toMap()["id"].toString()==mode)valid=true;}if(!valid){m_error=tr("Choose a connected display and one of its available modes.");emit changed();return;}
 QString runtime=QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);if(runtime.isEmpty()){m_error=tr("Cannot protect display changes without a runtime directory.");emit changed();return;}QDir().mkpath(runtime+"/harbor");m_transaction=runtime+"/harbor/layout-"+QUuid::createUuid().toString(QUuid::WithoutBraces);
 QJsonObject change{{"id",id},{"x",x},{"y",y},{"scale",scale},{"mode",mode},{"primary",primary}};
 if(!QProcess::startDetached("harbor-display-layout-guard",{"--directory",m_transaction,"--layout",QString::fromUtf8(QJsonDocument(change).toJson(QJsonDocument::Compact))})){m_error=tr("The display safety helper is not installed.");m_transaction.clear();emit changed();return;}
 m_busy=true;m_pending=false;m_transactionTimer.start();m_error.clear();m_poll.start();emit changed();
}
void DisplaySettings::poll(){
 QFile file(m_transaction+"/status.json");if(file.open(QIODevice::ReadOnly)){auto status=QJsonDocument::fromJson(file.readAll()).object();auto state=status["status"].toString();m_pending=state=="pending";
 if(state=="confirmed"||state=="reverted"||state=="failed"||state=="rollback-failed"){m_poll.stop();m_pending=false;m_busy=false;QString error=state=="confirmed"?QString():state=="reverted"?tr("Display changes were reverted. ")+status["error"].toString():status["error"].toString();m_transaction.clear();refresh();m_error=error;}
 }
 // A dead helper may leave a perfectly readable pending status forever. Bound the
 // whole transaction, including malformed/stale status, using monotonic time.
 if(!m_transaction.isEmpty()&&m_transactionTimer.hasExpired(m_transactionTimeoutMs)){m_poll.stop();m_busy=false;m_pending=false;m_transaction.clear();m_error=tr("The display safety helper timed out. The final display state is unknown; check the displays before retrying.");}
 emit changed();
}
void DisplaySettings::signalTransaction(const QString &name){if(!m_pending)return;QFile file(m_transaction+"/"+name);if(!file.open(QIODevice::WriteOnly)){m_error=tr("Could not contact the display safety helper.");emit changed();}}
void DisplaySettings::confirm(){signalTransaction("confirm");}void DisplaySettings::revert(){signalTransaction("revert");}
