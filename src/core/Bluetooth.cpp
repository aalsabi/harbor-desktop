#include "Bluetooth.h"
#include <QDBusMetaType>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDBusVariant>
#include <QDBusConnectionInterface>
#include <QRegularExpression>
#include <algorithm>
#include <memory>
Bluetooth::Bluetooth(QObject* parent,QDBusConnection connection):QObject(parent),bus(connection){
 qDBusRegisterMetaType<BluezInterfaces>();qDBusRegisterMetaType<BluezObjects>();
 values=summarize({});
 bus.connect("org.bluez",{},"org.freedesktop.DBus.ObjectManager","InterfacesAdded",this,SLOT(refresh()));
 bus.connect("org.bluez",{},"org.freedesktop.DBus.ObjectManager","InterfacesRemoved",this,SLOT(refresh()));
 bus.connect("org.bluez",{},"org.freedesktop.DBus.Properties","PropertiesChanged",this,SLOT(refresh()));
 auto watcher=new QDBusServiceWatcher("org.bluez",bus,QDBusServiceWatcher::WatchForOwnerChange,this);
 connect(watcher,&QDBusServiceWatcher::serviceOwnerChanged,this,[this]{scanAdapter.clear();scanWanted=false;agentRegistered=false;pairingPath.clear();if(agent)agent->dismiss();refresh();});
 scanTimer.setSingleShot(true);connect(&scanTimer,&QTimer::timeout,this,[this]{action("bluetooth-stop-scan");});
 QTimer::singleShot(0,this,&Bluetooth::refresh);
}
Bluetooth::~Bluetooth(){if(agentRegistered){auto m=QDBusMessage::createMethodCall("org.bluez","/org/bluez","org.bluez.AgentManager1","UnregisterAgent");m.setArguments({QVariant::fromValue(QDBusObjectPath("/org/harbor/BluetoothAgent"))});bus.asyncCall(m);}if(agent){agent->dismiss();bus.unregisterObject("/org/harbor/BluetoothAgent");}if(!scanAdapter.isEmpty()){auto m=QDBusMessage::createMethodCall("org.bluez",scanAdapter,"org.bluez.Adapter1","StopDiscovery");bus.asyncCall(m);}}
QVariantMap Bluetooth::summarize(const BluezObjects& objects){
 bool available=false,powered=false,discovering=false;QVariantList devices;QStringList connected;
 for(auto i=objects.begin();i!=objects.end();++i){
  if(i.value().contains("org.bluez.Adapter1")){available=true;auto a=i.value()["org.bluez.Adapter1"];powered|=a.value("Powered").toBool();discovering|=a.value("Discovering").toBool();}
  if(i.value().contains("org.bluez.Device1")){auto d=i.value()["org.bluez.Device1"];auto address=d.value("Address").toString();bool c=d.value("Connected").toBool();if(c)connected<<address;
   devices.append(QVariantMap{{"path",i.key().path()},{"address",address},{"name",d.value("Alias",d.value("Name",address))},{"connected",c},{"paired",d.value("Paired",false)},{"icon",d.value("Icon","bluetooth")}});
  }
 }
 std::sort(devices.begin(),devices.end(),[](const QVariant& a,const QVariant& b){auto x=a.toMap(),y=b.toMap();if(x["connected"]!=y["connected"])return x["connected"].toBool();return x["name"].toString().localeAwareCompare(y["name"].toString())<0;});
 return {{"bluetoothAvailable",available},{"bluetoothPowered",powered},{"bluetoothDiscovering",discovering},{"bluetoothDevices",devices},{"bluetoothConnected",connected},{"bluetoothStatus",!connected.isEmpty()?"Connected":powered?"On":"Off"},{"bluetooth",powered?"Powered: yes":"Powered: no"}};
}
void Bluetooth::refresh(){
 int request=++generation;auto m=QDBusMessage::createMethodCall("org.bluez","/","org.freedesktop.DBus.ObjectManager","GetManagedObjects");
 auto w=new QDBusPendingCallWatcher(bus.asyncCall(m,5000),this);
 connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,request]{QDBusPendingReply<BluezObjects> r=*w;w->deleteLater();if(request!=generation)return;
  objects=r.isError()?BluezObjects{}:r.value();adapter.clear();for(auto i=objects.begin();i!=objects.end();++i)if(i.value().contains("org.bluez.Adapter1")){if(adapter.isEmpty())adapter=i.key().path();if(i.value()["org.bluez.Adapter1"].value("Powered").toBool()){adapter=i.key().path();break;}}
  auto next=summarize(objects);next["bluetoothPrompt"]=values.value("bluetoothPrompt");next["bluetoothBusy"]=pending;next["bluetoothError"]=r.isError()?r.error().message():values.value("bluetoothError");values=next;emit changed();
 });
}
void Bluetooth::call(QString path,QString interface,QString method,QVariantList arguments,bool discovery){
 pending=true;values["bluetoothBusy"]=true;values["bluetoothError"]=QString();emit changed();
 auto m=QDBusMessage::createMethodCall("org.bluez",path,interface,method);m.setArguments(arguments);auto w=new QDBusPendingCallWatcher(bus.asyncCall(m,method=="Pair"?120000:25000),this);
 connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,path,discovery]{QDBusPendingReply<> r=*w;w->deleteLater();pending=false;if(!pairingPath.isEmpty()){pairingPath.clear();if(agent)agent->dismiss();}values["bluetoothBusy"]=false;values["bluetoothError"]=r.isError()?r.error().message():QString();
  if(discovery&&!r.isError()){scanAdapter=path;if(scanWanted)scanTimer.start(60000);else action("bluetooth-stop-scan");}
  emit changed();refresh();
 });
}
void Bluetooth::action(QString name,QVariant value){
 if(name=="bluetooth-respond"){auto reply=value.toMap();if(agent)agent->respond(reply.value("accept").toBool(),reply.value("value").toString());return;}
 if(name=="bluetooth-cancel-pair") { if(agent)agent->dismiss(); if(!pairingPath.isEmpty()){auto m=QDBusMessage::createMethodCall("org.bluez",pairingPath,"org.bluez.Device1","CancelPairing");bus.asyncCall(m);pairingPath.clear();} return; }

 if(name=="bluetooth-stop-scan"){
  scanWanted=false;scanTimer.stop();if(scanAdapter.isEmpty())return;auto path=scanAdapter;scanAdapter.clear();auto m=QDBusMessage::createMethodCall("org.bluez",path,"org.bluez.Adapter1","StopDiscovery");bus.asyncCall(m,5000);return;
 }
 if(pending)return;
 if(name=="bluetooth-scan"){scanWanted=true;if(!adapter.isEmpty()&&values["bluetoothPowered"].toBool()&&scanAdapter.isEmpty())call(adapter,"org.bluez.Adapter1","StartDiscovery",{},true);return;}
 if(name=="bluetooth"&&!adapter.isEmpty()){
  if(!value.toBool())action("bluetooth-stop-scan");
  QStringList paths;for(auto i=objects.begin();i!=objects.end();++i)if(i.value().contains("org.bluez.Adapter1"))paths<<i.key().path();
  pending=true;values["bluetoothBusy"]=true;values["bluetoothError"]=QString();emit changed();auto remaining=std::make_shared<int>(paths.size());
  for(auto path:paths){auto m=QDBusMessage::createMethodCall("org.bluez",path,"org.freedesktop.DBus.Properties","Set");m.setArguments({QString("org.bluez.Adapter1"),QString("Powered"),QVariant::fromValue(QDBusVariant(value.toBool()))});auto w=new QDBusPendingCallWatcher(bus.asyncCall(m,10000),this);
   connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,remaining]{QDBusPendingReply<> r=*w;w->deleteLater();if(r.isError())values["bluetoothError"]=r.error().message();if(--*remaining==0){pending=false;values["bluetoothBusy"]=false;emit changed();refresh();}});
  }return;
 }
 if(name=="bluetooth-pair"||name=="bluetooth-unpair") {
  const QString path=value.toString();if(!QRegularExpression("^/org/bluez/[A-Za-z0-9_/]+$").match(path).hasMatch())return;const auto entry=objects.value(QDBusObjectPath(path));
  if(!entry.contains("org.bluez.Device1")) {values["bluetoothError"]=tr("Select an available Bluetooth device.");emit changed();return;}
  if(name=="bluetooth-pair") {pairDevice(path);return;}
  const auto adapterPath=qvariant_cast<QDBusObjectPath>(entry.value("org.bluez.Device1").value("Adapter")).path();
  if(!objects.value(QDBusObjectPath(adapterPath)).contains("org.bluez.Adapter1"))return;
  call(adapterPath,"org.bluez.Adapter1","RemoveDevice",{QVariant::fromValue(QDBusObjectPath(path))});return;
 }
 if(name=="bluetooth-connect"||name=="bluetooth-disconnect")for(auto row:values["bluetoothDevices"].toList()){auto d=row.toMap();if(d["address"].toString()==value.toString()){call(d["path"].toString(),"org.bluez.Device1",name=="bluetooth-connect"?"Connect":"Disconnect");return;}}
}

void Bluetooth::prompt(const QString &kind,const QString &device,const QString &code) {
 values["bluetoothPrompt"]=QVariantMap{{"kind",kind},{"path",device},{"code",code},{"name",objects.value(QDBusObjectPath(device)).value("org.bluez.Device1").value("Alias",device)}};emit changed();
}
void Bluetooth::clearPrompt(){values["bluetoothPrompt"]=QVariantMap{};emit changed();}
void Bluetooth::pairDevice(const QString &path){
 pairingPath=path;
 if(agentRegistered){pairingPath=path;call(path,"org.bluez.Device1","Pair");return;}
 if(!agent){agent=new BluetoothAgent(this);if(!bus.registerObject("/org/harbor/BluetoothAgent",agent,QDBusConnection::ExportAllSlots)){values["bluetoothError"]=tr("Could not register the pairing agent.");agent->deleteLater();agent=nullptr;emit changed();return;}}
 pending=true;values["bluetoothBusy"]=true;values["bluetoothError"]=QString();emit changed();
 auto m=QDBusMessage::createMethodCall("org.bluez","/org/bluez","org.bluez.AgentManager1","RegisterAgent");m.setArguments({QVariant::fromValue(QDBusObjectPath("/org/harbor/BluetoothAgent")),QString("KeyboardDisplay")});
 auto w=new QDBusPendingCallWatcher(bus.asyncCall(m),this);connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,path]{QDBusPendingReply<> reply=*w;w->deleteLater();pending=false;values["bluetoothBusy"]=false;if(reply.isError()){values["bluetoothError"]=reply.error().message();emit changed();return;}agentRegistered=true;if(pairingPath!=path){emit changed();return;}call(path,"org.bluez.Device1","Pair");});
}
BluetoothAgent::BluetoothAgent(Bluetooth *value):QObject(value),owner(value){}
bool BluetoothAgent::trustedCaller() const {return calledFromDBus() && owner->bus.interface() && message().service()==owner->bus.interface()->serviceOwner("org.bluez").value();}
bool BluetoothAgent::request(const QString &kind,const QDBusObjectPath &device,const QString &code){
 if(!trustedCaller() || device.path()!=owner->pairingPath || hasWaiting){if(calledFromDBus())sendErrorReply("org.bluez.Error.Rejected","No matching user-initiated pairing request.");return false;}
 setDelayedReply(true);waiting=message();hasWaiting=true;requestKind=kind;owner->prompt(kind,device.path(),code);return true;
}
void BluetoothAgent::respond(bool accept,const QString &value){
 if(!hasWaiting)return;
 if(accept && requestKind=="pin" && (value.isEmpty() || value.size()>16 || !QRegularExpression("^[ -~]+$").match(value).hasMatch())){owner->values["bluetoothError"]=tr("Enter a PIN of 1–16 characters.");emit owner->changed();return;}
 if(accept && requestKind=="passkey" && !QRegularExpression("^[0-9]{1,6}$").match(value).hasMatch()){owner->values["bluetoothError"]=tr("Enter a passkey of up to six digits.");emit owner->changed();return;}
 if(!accept)owner->bus.send(waiting.createErrorReply("org.bluez.Error.Rejected","Pairing rejected by the user."));
 else {QVariantList args;if(requestKind=="pin")args<<value;else if(requestKind=="passkey")args<<QVariant::fromValue(value.toUInt());owner->bus.send(waiting.createReply(args));}
 hasWaiting=false;waiting=QDBusMessage();requestKind.clear();owner->clearPrompt();
}
void BluetoothAgent::dismiss(){respond(false);owner->clearPrompt();}
void BluetoothAgent::Release(){if(trustedCaller()){dismiss();owner->agentRegistered=false;}}
QString BluetoothAgent::RequestPinCode(const QDBusObjectPath &device){request("pin",device);return {};}
uint BluetoothAgent::RequestPasskey(const QDBusObjectPath &device){request("passkey",device);return 0;}
void BluetoothAgent::DisplayPinCode(const QDBusObjectPath &device,const QString &code){if(trustedCaller() && device.path()==owner->pairingPath)owner->prompt("display",device.path(),code);else if(calledFromDBus())sendErrorReply("org.bluez.Error.Rejected","Unexpected device.");}
void BluetoothAgent::DisplayPasskey(const QDBusObjectPath &device,uint code,ushort entered){Q_UNUSED(entered);DisplayPinCode(device,QString::number(code).rightJustified(6,'0'));}
void BluetoothAgent::RequestConfirmation(const QDBusObjectPath &device,uint code){request("confirm",device,QString::number(code).rightJustified(6,'0'));}
void BluetoothAgent::RequestAuthorization(const QDBusObjectPath &device){request("authorize",device);}
void BluetoothAgent::AuthorizeService(const QDBusObjectPath &device,const QString &uuid){request("authorize",device,uuid);}
void BluetoothAgent::Cancel(){if(trustedCaller())dismiss();}
