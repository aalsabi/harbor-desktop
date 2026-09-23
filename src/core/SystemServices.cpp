#include "SystemServices.h"
#include <QStandardPaths>
#include <QProcess>
#include <QTimer>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <cmath>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QUuid>
#include <memory>
SystemServices::SystemServices(QObject* p):QObject(p){values["displayOutputs"]=QVariantList{};values["displayPending"]=false;values["displayChanging"]=false;for(auto key:{"wifi","network","networks","volume","audio","bluetooth","devices","brightness","power","displays"})values[QString(key)+"Available"]=false;QTimer::singleShot(0,this,&SystemServices::refresh);auto t=new QTimer(this);connect(t,&QTimer::timeout,this,&SystemServices::refresh);t->start(15000);}
void SystemServices::query(QString key,QString program,QStringList args){if(QStandardPaths::findExecutable(program).isEmpty()){values[key+"Available"]=false;emit changed();return;}auto c=new Command(this);connect(c,&Command::finished,this,[this,c,key](bool ok,QString out){values[key+"Available"]=ok;values[key]=ok?out:QString();if(key=="displays"&&ok)values["displayOutputs"]=QJsonDocument::fromJson(out.toUtf8()).object()["outputs"].toArray().toVariantList();c->deleteLater();emit changed();});c->run(program,args);}
void SystemServices::refresh(){query("wifi","nmcli",{"radio","wifi"});query("network","nmcli",{"-t","-f","NAME,TYPE,DEVICE","connection","show","--active"});query("networks","nmcli",{"-t","-f","SSID,SIGNAL,SECURITY","device","wifi","list","--rescan","no"});query("volume","wpctl",{"get-volume","@DEFAULT_AUDIO_SINK@"});query("audio","wpctl",{"status"});query("bluetooth","bluetoothctl",{"show"});query("devices","bluetoothctl",{"devices"});query("brightness","brightnessctl",{"-m"});query("power","powerprofilesctl",{"get"});query("displays","kscreen-doctor",{"-j"});}
void SystemServices::execute(QString program,QStringList args){if(busy())return;auto c=new Command(this);++active;status.clear();emit changed();connect(c,&Command::finished,this,[this,c](bool ok,QString out){--active;status=ok?tr("Applied; refreshing device state"):out;c->deleteLater();refresh();emit changed();});c->run(program,args,10000);}
void SystemServices::action(QString name,QVariant v){
 if(name=="wifi")execute("nmcli",{"radio","wifi",v.toBool()?"on":"off"});
 else if(name=="volume"){double n=v.toDouble();if(std::isfinite(n)&&n>=0&&n<=1)execute("wpctl",{"set-volume","@DEFAULT_AUDIO_SINK@",QString::number(n)});}
 else if(name=="mute")execute("wpctl",{"set-mute","@DEFAULT_AUDIO_SINK@","toggle"});
 else if(name=="bluetooth")execute("bluetoothctl",{"power",v.toBool()?"on":"off"});
 else if(name=="brightness"){int n=v.toInt();if(n>=5&&n<=100)execute("brightnessctl",{"set",QString::number(n)+"%"});}
 else if(name=="power"&&QStringList{"power-saver","balanced","performance"}.contains(v.toString()))execute("powerprofilesctl",{"set",v.toString()});
 else if(name=="lock"){
  auto msg=QDBusMessage::createMethodCall("org.freedesktop.ScreenSaver","/ScreenSaver","org.freedesktop.ScreenSaver","Lock");
  auto watcher=new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(msg,5000),this);
  connect(watcher,&QDBusPendingCallWatcher::finished,this,[this,watcher]{QDBusPendingReply<> r=*watcher;status=r.isError()?tr("Lock service unavailable: ")+r.error().message():tr("Lock requested");emit changed();watcher->deleteLater();});
 }else {status=tr("Unsupported action");emit changed();}
}
void SystemServices::openTool(QString name){QMap<QString,QStringList> tools{{"network",{"nm-connection-editor"}},{"bluetooth",{"blueman-manager"}},{"audio",{"pavucontrol"}},{"users",{"user-manager"}},{"updates",{"plasma-discover"}},{"terminal",{"x-terminal-emulator"}},{"files",{"harbor-files"}},{"settings",{"harbor-settings"}}};if(!tools.contains(name))return;auto args=tools[name];auto cmd=args.takeFirst();if(!QProcess::startDetached(cmd,args)){status=tr("Install the external tool: ")+cmd;emit changed();}}

void SystemServices::applyDisplay(int output,double scale,QString mode){
 if(!displayTransaction.isEmpty()||!std::isfinite(scale)||scale<.5||scale>4)return;
 auto runtime=QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);if(runtime.isEmpty()){status=tr("No runtime directory; cannot guard display change");emit changed();return;}
 QDir().mkpath(runtime+"/harbor");
 auto path=runtime+"/harbor/display-"+QUuid::createUuid().toString(QUuid::WithoutBraces);
 QStringList args{"--directory",path,"--output",QString::number(output),"--scale",QString::number(scale)};
 if(!mode.isEmpty())args<< "--mode"<<mode;
 if(!QProcess::startDetached("harbor-display-guard",args)){status=tr("Display guard is not installed");emit changed();return;}
 displayTransaction=path;values["displayPending"]=false;values["displayChanging"]=true;
 displayPoll=new QTimer(this);auto ticks=std::make_shared<int>(0);
 connect(displayPoll,&QTimer::timeout,this,[this,ticks]{
  QFile f(displayTransaction+"/status.json");
  if(f.open(QIODevice::ReadOnly)){
   auto o=QJsonDocument::fromJson(f.readAll()).object();auto state=o["status"].toString();
   values["displayPending"]=state=="pending";status=state=="pending"?tr("Keep these settings? Automatic rollback after 15 seconds."):state+" "+o["error"].toString();
   if(state!="pending"){displayPoll->stop();displayPoll->deleteLater();displayPoll=nullptr;displayTransaction.clear();values["displayChanging"]=false;refresh();}
  }else if(++*ticks>80){displayPoll->stop();displayPoll->deleteLater();displayPoll=nullptr;displayTransaction.clear();values["displayChanging"]=false;status=tr("Display guard did not report; check the display manually");}
  emit changed();
 });displayPoll->start(250);emit changed();
}
void SystemServices::confirmDisplay(){if(displayTransaction.isEmpty()||!values["displayPending"].toBool())return;QFile f(displayTransaction+"/confirm");if(!f.open(QIODevice::WriteOnly)){status=tr("Could not confirm; display will revert");emit changed();}}
