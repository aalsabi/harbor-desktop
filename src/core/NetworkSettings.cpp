#include "NetworkSettings.h"
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusMetaType>
#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QDBusVariant>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QHostAddress>
#include <QRegularExpression>
#include <QUuid>
#include <QProcess>
#include <QFileInfo>
#include <QUrl>
#include <QTimer>
#include <pwd.h>
#include <unistd.h>
namespace {
const QString nm="org.freedesktop.NetworkManager",base="/org/freedesktop/NetworkManager",settingsIface=nm+".Settings.Connection";
QString username(){const auto *entry=getpwuid(getuid());return entry?QString::fromLocal8Bit(entry->pw_name):QString();}
QVariantMap props(const QDBusMessage &reply){return reply.arguments().isEmpty()?QVariantMap():qdbus_cast<QVariantMap>(reply.arguments().first());}
QList<QDBusObjectPath> paths(const QVariant &value){return qdbus_cast<QList<QDBusObjectPath>>(value);}
NetworkProfile profile(const QDBusMessage &reply){return reply.arguments().isEmpty()?NetworkProfile():qdbus_cast<NetworkProfile>(reply.arguments().first());}
QStringList parts(const QString &input){return input.split(QRegularExpression("[,\\s]+"),Qt::SkipEmptyParts);}
QString certificatePath(const QVariant &value){auto bytes=value.toByteArray();if(!bytes.startsWith("file://")||!bytes.endsWith('\0'))return {};return QString::fromUtf8(bytes.mid(7,bytes.size()-8));}
QString enterpriseSettings(const QVariantMap &draft,QVariantMap &eap){
 const QString method=draft.value("eap","peap").toString();
 if(!QStringList{"tls","peap","ttls"}.contains(method))return "Choose TLS, PEAP, or TTLS authentication.";
 if(draft.value("identity").toString().trimmed().isEmpty())return "Enter an enterprise identity.";
 const QString domain=draft.value("domainSuffixMatch").toString().trimmed();
 const QRegularExpression domainPattern("^(?=.{1,253}$)[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?(?:\\.[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)+$");
 if(!domainPattern.match(domain).hasMatch())return "Enter the authentication server domain supplied by your administrator.";
 for(const auto &entry:QList<QPair<QString,QString>>{{"caCert","ca-cert"},{"clientCert","client-cert"},{"privateKey","private-key"}}){
  if(method!="tls"&&entry.first!="caCert")continue;
  const QString input=draft.value(entry.first).toString();
  if(input.isEmpty()){if(eap.value(entry.second).toByteArray().isEmpty())return "Select the required CA certificate, client certificate, and private key files.";continue;}
  QUrl url(input);QString path=url.isLocalFile()?url.toLocalFile():input;QFileInfo info(path);
  if(path.contains(QChar(0))||!info.isAbsolute()||!info.isFile()||!info.isReadable()||info.size()>5*1024*1024)return "Select readable local certificate and key files (up to 5 MB).";
  eap[entry.second]=QByteArray("file://")+info.absoluteFilePath().toUtf8()+QByteArray(1,'\0');
 }
 if(method!="tls"){
  const auto inner=draft.value("phase2Auth","mschapv2").toString();
  if(!(method=="peap"?QStringList{"mschapv2","gtc"}:QStringList{"pap","chap","mschap","mschapv2"}).contains(inner))return "Choose an inner authentication method supported by this EAP type.";
  eap["phase2-auth"]=inner;eap.remove("phase2-autheap");
  if(!draft.value("eapPassword").toString().isEmpty()){eap["password"]=draft.value("eapPassword");eap["password-flags"]=uint(0);}
 }else{eap.remove("phase2-auth");eap.remove("phase2-autheap");}
 if(method=="tls"&&!draft.value("privateKeyPassword").toString().isEmpty()){eap["private-key-password"]=draft.value("privateKeyPassword");eap["private-key-password-flags"]=uint(0);}
 eap["eap"]=QStringList{method};eap["identity"]=draft.value("identity").toString();eap["anonymous-identity"]=draft.value("anonymousIdentity").toString();eap["domain-suffix-match"]=domain;eap["system-ca-certs"]=false;eap["optional"]=false;
 // Never retain a legacy flag that disables server certificate time checks.
 eap["phase1-auth-flags"]=eap.value("phase1-auth-flags").toUInt()&~uint(8);
 return {};
}
QVariantMap ipSettings(const QVariantMap &draft,int family){
 const QString prefix=family==4?"ipv4":"ipv6";const QString method=draft.value(prefix+"Method","auto").toString();
 QVariantMap result{{"method",method},{"ignore-auto-dns",draft.value(prefix+"ManualDns").toBool()}};
 NetworkAddresses addresses;
 if(method=="manual")for(const auto &item:parts(draft.value(prefix+"Addresses").toString())){const auto bits=item.split('/');addresses.append({{"address",bits[0]},{"prefix",QVariant::fromValue(bits[1].toUInt())}});}
 result["address-data"]=QVariant::fromValue(addresses);
 result["gateway"]=method=="manual"?draft.value(prefix+"Gateway").toString():QString();
 result["dns-data"]=method=="disabled"?QStringList():parts(draft.value(prefix+"Dns").toString());return result;
}
}
NetworkSettings::NetworkSettings(QObject *parent,QDBusConnection bus,QString importHelper):QObject(parent),m_bus(bus),m_importHelper(importHelper){qDBusRegisterMetaType<NetworkProfile>();qDBusRegisterMetaType<NetworkAddresses>();m_secretTimer.setSingleShot(true);connect(&m_secretTimer,&QTimer::timeout,this,&NetworkSettings::cancelSecrets);m_refreshTimer.setInterval(5000);connect(&m_refreshTimer,&QTimer::timeout,this,&NetworkSettings::refresh);qDBusRegisterMetaType<QMap<QString,QString>>();auto *watcher=new QDBusServiceWatcher(nm,m_bus,QDBusServiceWatcher::WatchForOwnerChange,this);connect(watcher,&QDBusServiceWatcher::serviceOwnerChanged,this,[this]{m_agentRegistered=false;cancelSecrets();});}
NetworkSettings::~NetworkSettings(){cancelSecrets();if(m_agentRegistered){auto message=QDBusMessage::createMethodCall(nm,base+"/AgentManager",nm+".AgentManager","Unregister");m_bus.asyncCall(message);}if(m_agent)m_bus.unregisterObject("/org/harbor/NetworkAgent");}
void NetworkSettings::respondSecrets(bool accept,const QVariantMap &values){if(m_agent)m_agent->respond(accept,values);}
void NetworkSettings::cancelSecrets(){m_secretTimer.stop();if(m_agent)m_agent->respond(false,{});m_activatingPath.clear();}
void NetworkSettings::fail(const QString &message){m_error=message;emit changed();}
void NetworkSettings::read(const QString &path,const QString &iface,const QString &method,const QVariantList &args,std::function<void(const QDBusMessage&)> done){
 ++m_reads;auto msg=QDBusMessage::createMethodCall(nm,path,iface,method);msg.setArguments(args);auto *watcher=new QDBusPendingCallWatcher(m_bus.asyncCall(msg,15000),this);
 connect(watcher,&QDBusPendingCallWatcher::finished,this,[this,watcher,done]{const auto reply=watcher->reply();watcher->deleteLater();if(reply.type()==QDBusMessage::ErrorMessage)m_error=reply.errorMessage();done(reply);if(--m_reads==0){m_busy=false;emit changed();}});
}
void NetworkSettings::write(const QString &path,const QString &iface,const QString &method,const QVariantList &args,std::function<void(const QDBusMessage&)> done){
 m_busy=true;m_error.clear();emit changed();auto msg=QDBusMessage::createMethodCall(nm,path,iface,method);msg.setArguments(args);auto *watcher=new QDBusPendingCallWatcher(m_bus.asyncCall(msg,120000),this);
 connect(watcher,&QDBusPendingCallWatcher::finished,this,[this,watcher,done]{const auto reply=watcher->reply();watcher->deleteLater();m_busy=false;if(reply.type()==QDBusMessage::ErrorMessage){m_error=reply.errorMessage();emit changed();return;}if(done)done(reply);else refresh();});
}
void NetworkSettings::setActive(bool active){if(active){refresh();m_refreshTimer.start();}else{m_refreshTimer.stop();cancelSecrets();}}
void NetworkSettings::refresh(){
 if(m_busy)return;m_busy=true;m_error.clear();emit changed();
 read(base,"org.freedesktop.DBus.Properties","GetAll",{nm},[this](const QDBusMessage &reply){
  if(reply.type()==QDBusMessage::ErrorMessage){m_available=false;return;}m_available=true;const auto values=props(reply);m_wireless=values.value("WirelessEnabled").toBool();
  m_devices.clear();m_networks.clear();m_profiles.clear();m_configs.clear();
  for(const auto &path:paths(values.value("Devices")))loadDevice(path.path());
  read(base+"/Settings",nm+".Settings","ListConnections",{},[this](const QDBusMessage &r){if(r.type()!=QDBusMessage::ErrorMessage&&!r.arguments().isEmpty())for(const auto &p:paths(r.arguments().first()))loadProfile(p.path());});
 });
}
void NetworkSettings::loadDevice(const QString &path){
 read(path,"org.freedesktop.DBus.Properties","GetAll",{nm+".Device"},[this,path](const QDBusMessage &reply){
  if(reply.type()==QDBusMessage::ErrorMessage)return;auto p=props(reply);const uint type=p.value("DeviceType").toUInt();if(type!=1&&type!=2)return;
  m_devices.append(QVariantMap{{"path",path},{"interface",p.value("Interface")},{"type",type==2?"wifi":"ethernet"},{"state",p.value("State")},{"managed",p.value("Managed")}});
  if(type==2)read(path,"org.freedesktop.DBus.Properties","GetAll",{nm+".Device.Wireless"},[this,path](const QDBusMessage &reply){if(reply.type()==QDBusMessage::ErrorMessage)return;for(const auto &ap:paths(props(reply).value("AccessPoints")))read(ap.path(),"org.freedesktop.DBus.Properties","GetAll",{nm+".AccessPoint"},[this,path,ap](const QDBusMessage &r){if(r.type()==QDBusMessage::ErrorMessage)return;auto p=props(r);const uint security=p.value("RsnFlags").toUInt()|p.value("WpaFlags").toUInt();const bool privacy=p.value("Flags").toUInt()&1;m_networks.append(QVariantMap{{"path",ap.path()},{"device",path},{"ssid",p.value("Ssid").toByteArray()},{"name",QString::fromUtf8(p.value("Ssid").toByteArray())},{"strength",p.value("Strength")},{"security",security},{"secured",privacy||security!=0},{"enterprise",bool(security&0x200)},{"sae",bool(security&0x400)}});});});
 });
}
void NetworkSettings::loadProfile(const QString &path){read(path,settingsIface,"GetSettings",{},[this,path](const QDBusMessage &reply){if(reply.type()==QDBusMessage::ErrorMessage)return;const auto config=profile(reply);m_configs[path]=config;const auto connection=config.value("connection");m_profiles.append(QVariantMap{{"path",path},{"name",connection.value("id")},{"type",connection.value("type")},{"uuid",connection.value("uuid")}});});}
bool NetworkSettings::knownDevice(const QString &path)const{for(const auto &v:m_devices)if(v.toMap().value("path").toString()==path)return true;return false;}
void NetworkSettings::setWirelessEnabled(bool enabled){if(m_busy||!m_available)return;write(base,"org.freedesktop.DBus.Properties","Set",{nm,QString("WirelessEnabled"),QVariant::fromValue(QDBusVariant(enabled))});}
void NetworkSettings::scan(){if(m_busy)return;for(const auto &d:m_devices){const auto device=d.toMap();if(device.value("type")=="wifi"){write(device.value("path").toString(),nm+".Device.Wireless","RequestScan",{QVariantMap{}});return;}}fail(tr("No Wi-Fi adapter is available."));}
void NetworkSettings::connectWifi(const QString &ap,const QString &password){
 if(m_busy)return;QVariantMap chosen;for(const auto &v:m_networks)if(v.toMap().value("path").toString()==ap)chosen=v.toMap();if(chosen.isEmpty()){fail(tr("This network is no longer available. Refresh the list."));return;}
 const uint security=chosen.value("security").toUInt();if(chosen.value("enterprise").toBool()){fail(tr("Enterprise Wi-Fi requires a preconfigured 802.1X profile. Select it under Saved connections."));return;}
 if(chosen.value("secured").toBool() && !(security&0x100) && !(security&0x400)){fail(tr("This Wi-Fi security type is unsupported. Use a configured connection profile."));return;}
 const bool sae=chosen.value("sae").toBool();
 if(chosen.value("secured").toBool() && (sae ? (password.isEmpty() || password.size()>63) : ((password.size()<8 || password.size()>63) && !(password.size()==64&&QRegularExpression("^[0-9a-fA-F]{64}$").match(password).hasMatch())))){fail(tr("Enter a Wi-Fi password of 8–63 characters or a 64-digit hexadecimal key."));return;}
 if(username().isEmpty()){fail(tr("Could not determine the current user."));return;}
 NetworkProfile config{{"connection",{{"id",chosen.value("name")},{"uuid",QUuid::createUuid().toString(QUuid::WithoutBraces)},{"type",QString("802-11-wireless")},{"permissions",QStringList{"user:"+username()+":"}}}},{"802-11-wireless",{{"ssid",chosen.value("ssid")}}},{"ipv4",{{"method",QString("auto")}}},{"ipv6",{{"method",QString("auto")}}}};
 if(chosen.value("secured").toBool())config["802-11-wireless-security"]={{"key-mgmt",QString(chosen.value("sae").toBool()?"sae":"wpa-psk")},{"psk",password}};
 write(base,nm,"AddAndActivateConnection",{QVariant::fromValue(config),QVariant::fromValue(QDBusObjectPath(chosen.value("device").toString())),QVariant::fromValue(QDBusObjectPath(ap))});
}
void NetworkSettings::activate(const QString &path){if(m_busy)return;if(!m_configs.contains(path)){fail(tr("Select a saved connection."));return;}m_activatingPath=path;activateWithAgent(path);}
void NetworkSettings::disconnectDevice(const QString &path){if(m_busy)return;if(!knownDevice(path)){fail(tr("Select an available network device."));return;}write(path,nm+".Device","Disconnect",{});}
QString NetworkSettings::validateIp(const QVariantMap &draft,int family){
 const QString key=family==4?"ipv4":"ipv6",method=draft.value(key+"Method","auto").toString();if(method!="auto"&&method!="manual"&&method!="disabled")return "Choose automatic, manual, or disabled addressing.";
 const auto protocol=family==4?QAbstractSocket::IPv4Protocol:QAbstractSocket::IPv6Protocol;
 const auto addresses=parts(draft.value(key+"Addresses").toString());if(method=="manual"&&addresses.isEmpty())return "Manual addressing requires an address and prefix length.";
 if(method=="manual")for(const auto &address:addresses){const auto tokens=address.split('/');bool valid=false;const int prefix=tokens.size()==2?tokens[1].toInt(&valid):-1;if(tokens.size()!=2||!valid||prefix<0||prefix>(family==4?32:128)||QHostAddress(tokens[0]).protocol()!=protocol)return "Enter addresses with a valid prefix, such as 192.168.1.10/24 or 2001:db8::10/64.";}
 const QString gateway=draft.value(key+"Gateway").toString();if(method=="manual"&&!gateway.isEmpty()&&QHostAddress(gateway).protocol()!=protocol)return "The gateway must match the address family.";
 for(const auto &dns:parts(draft.value(key+"Dns").toString()))if(QHostAddress(dns).protocol()!=protocol)return "DNS servers must be valid addresses for the selected IP family.";
 if(draft.value(key+"ManualDns").toBool()&&parts(draft.value(key+"Dns").toString()).isEmpty()&&method!="disabled")return "Enter at least one DNS server or enable automatic DNS.";
 return {};
}
QVariantMap NetworkSettings::profileDraft(const QString &path)const{
 const auto config=m_configs.value(path);QVariantMap draft{{"name",config.value("connection").value("id")},{"type",config.value("connection").value("type","802-3-ethernet")},{"interface",config.value("connection").value("interface-name")}};
 for(const QString family:{QString("ipv4"),QString("ipv6")}){const auto ip=config.value(family);QStringList addresses;for(const auto &a:qdbus_cast<NetworkAddresses>(ip.value("address-data")))addresses<<a.value("address").toString()+"/"+a.value("prefix").toString();draft[family+"Method"]=ip.value("method","auto");draft[family+"Addresses"]=addresses.join(", ");draft[family+"Gateway"]=ip.value("gateway");draft[family+"ManualDns"]=ip.value("ignore-auto-dns",false);draft[family+"Dns"]=ip.value("dns-data").toStringList().join(", ");}
 if(config.contains("vpn")){const auto data=qdbus_cast<QMap<QString,QString>>(config.value("vpn").value("data"));draft["vpnUsername"]=data.value("username");draft["vpnOpenvpn"]=config.value("vpn").value("service-type")=="org.freedesktop.NetworkManager.openvpn";}
 if(config.contains("802-1x")){
  const auto eap=config.value("802-1x");const auto methods=eap.value("eap").toStringList();draft["enterprise"]=true;draft["enterpriseEditable"]=methods.size()==1&&QStringList{"tls","peap","ttls"}.contains(methods.first())&&!eap.contains("phase2-autheap");draft["eap"]=methods.value(0);
  for(const auto &entry:QList<QPair<QString,QString>>{{"identity","identity"},{"anonymousIdentity","anonymous-identity"},{"phase2Auth","phase2-auth"},{"domainSuffixMatch","domain-suffix-match"}})draft[entry.first]=eap.value(entry.second);
  for(const auto &entry:QList<QPair<QString,QString>>{{"caCert","ca-cert"},{"clientCert","client-cert"},{"privateKey","private-key"}})draft[entry.first]=certificatePath(eap.value(entry.second));
 }
 draft["ssid"]=QString::fromUtf8(config.value("802-11-wireless").value("ssid").toByteArray());
 return draft;
}
void NetworkSettings::saveProfile(const QString &path,const QVariantMap &draft){
 if(m_busy)return;if(!path.isEmpty()&&!m_configs.contains(path)){fail(tr("The selected profile is no longer available."));return;}
 if(draft.value("name").toString().trimmed().isEmpty()){fail(tr("Enter a connection name."));return;}
 for(const int family:{4,6}){const auto error=validateIp(draft,family);if(!error.isEmpty()){fail(error);return;}}
 if(draft.value("ipv4Method","auto")=="disabled"&&draft.value("ipv6Method","auto")=="disabled"){fail(tr("Enable IPv4 or IPv6."));return;}
 NetworkProfile config=m_configs.value(path);auto connection=config.value("connection");connection["id"]=draft.value("name").toString().trimmed();
 if(path.isEmpty()){if(username().isEmpty()){fail(tr("Could not determine the current user."));return;}connection["uuid"]=QUuid::createUuid().toString(QUuid::WithoutBraces);connection["type"]=draft.value("type","802-3-ethernet").toString();connection["permissions"]=QStringList{"user:"+username()+":"};config["802-3-ethernet"]={};if(!draft.value("interface").toString().isEmpty())connection["interface-name"]=draft.value("interface");}
 config["connection"]=connection;
 if(path.isEmpty()&&connection.value("type")=="802-11-wireless"){
  const auto ssid=draft.value("ssid").toString().toUtf8();if(ssid.isEmpty()||ssid.size()>32||!draft.value("enterprise").toBool()){fail(tr("Enter an enterprise Wi-Fi network name of 1–32 bytes."));return;}
  config.remove("802-3-ethernet");config["802-11-wireless"]={{"ssid",ssid},{"mode",QString("infrastructure")}};
 }
 if(path.isEmpty()&&connection.value("type")!="802-11-wireless"&&connection.value("type")!="802-3-ethernet"){fail(tr("Choose a wired or Wi-Fi connection."));return;}
 if(draft.value("enterprise").toBool()&&draft.value("enterpriseEditable",true).toBool()){
  auto eap=config.value("802-1x");const auto error=enterpriseSettings(draft,eap);if(!error.isEmpty()){fail(error);return;}config["802-1x"]=eap;
  if(connection.value("type")=="802-11-wireless"&&!config.value("802-11-wireless-security").contains("key-mgmt"))config["802-11-wireless-security"]["key-mgmt"]=QString("wpa-eap");
 }
 if(config.value("vpn").value("service-type")=="org.freedesktop.NetworkManager.openvpn"){
  auto data=qdbus_cast<QMap<QString,QString>>(config.value("vpn").value("data"));data["username"]=draft.value("vpnUsername").toString();config["vpn"]["data"]=QVariant::fromValue(data);
  if(!draft.value("vpnPassword").toString().isEmpty()){config["vpn"]["secrets"]=QVariant::fromValue(QMap<QString,QString>{{"password",draft.value("vpnPassword").toString()}});data["password-flags"]="0";config["vpn"]["data"]=QVariant::fromValue(data);}
 }
 for(int family:{4,6}){const QString key=family==4?"ipv4":"ipv6";auto ip=config.value(key);ip.remove("addresses");ip.remove("dns");const auto updated=ipSettings(draft,family);for(auto i=updated.begin();i!=updated.end();++i)ip[i.key()]=i.value();config[key]=ip;}
 if(path.isEmpty())write(base+"/Settings",nm+".Settings","AddConnection",{QVariant::fromValue(config)},[this](const QDBusMessage &reply){emit profileSaved(qdbus_cast<QDBusObjectPath>(reply.arguments().first()).path());refresh();});else updatePreservingSecrets(path,config,{"802-11-wireless-security","802-1x","vpn","wireguard"});
}
void NetworkSettings::importVpn(const QString &file,const QString &type){
 if(m_busy)return;QUrl url(file);const QString path=url.isLocalFile()?url.toLocalFile():file;QFileInfo info(path);
 if((type!="openvpn"&&type!="wireguard")||!info.isAbsolute()||!info.isFile()||!info.isReadable()||info.size()>5*1024*1024){fail(tr("Select a readable local OpenVPN or WireGuard configuration file (up to 5 MB)."));return;}
 m_busy=true;m_error.clear();emit changed();auto *process=new QProcess(this);auto env=QProcessEnvironment::systemEnvironment();env.insert("LC_ALL","C");process->setProcessEnvironment(env);process->setProgram(m_importHelper);process->setArguments({type,info.absoluteFilePath()});
 connect(process,&QProcess::errorOccurred,this,[this,process](QProcess::ProcessError e){if(e==QProcess::FailedToStart){m_busy=false;fail(tr("The Harbor VPN import helper is unavailable."));process->deleteLater();}});
 connect(process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this,process](int code,QProcess::ExitStatus status){process->readAllStandardOutput();process->readAllStandardError();process->deleteLater();m_busy=false;if(code!=0||status!=QProcess::NormalExit){fail(tr("VPN import failed. Check the configuration, NetworkManager authorization, and installed VPN plugin (python3-gi and gir1.2-nm-1.0 are required)."));return;}refresh();});
 process->start();QTimer::singleShot(40000,process,[process]{if(process->state()!=QProcess::NotRunning)process->kill();});
}
void NetworkSettings::updatePreservingSecrets(const QString &path,NetworkProfile config,QStringList keys){
 while(!keys.isEmpty()&&!config.contains(keys.first()))keys.removeFirst();
 if(keys.isEmpty()){write(path,settingsIface,"Update",{QVariant::fromValue(config)},[this,path](const QDBusMessage &){emit profileSaved(path);refresh();});return;}
 const QString key=keys.takeFirst();
 write(path,settingsIface,"GetSecrets",{key},[this,path,config,keys](const QDBusMessage &reply) mutable {const auto secrets=profile(reply);for(auto group=secrets.begin();group!=secrets.end();++group)for(auto entry=group.value().begin();entry!=group.value().end();++entry){if(group.key()=="vpn"&&entry.key()=="secrets"){auto merged=qdbus_cast<QMap<QString,QString>>(entry.value());const auto fresh=qdbus_cast<QMap<QString,QString>>(config.value("vpn").value("secrets"));for(auto item=fresh.begin();item!=fresh.end();++item)merged[item.key()]=item.value();config["vpn"]["secrets"]=QVariant::fromValue(merged);}else if(!config.value(group.key()).contains(entry.key()))config[group.key()][entry.key()]=entry.value();}updatePreservingSecrets(path,config,keys);});
}

void NetworkSettings::activateWithAgent(const QString &path){
 if(m_activatingPath!=path){emit changed();return;}
 if(!m_agent){m_agent=new NetworkSecretAgent(this);if(!m_bus.registerObject("/org/harbor/NetworkAgent",m_agent,QDBusConnection::ExportAllSlots)){m_agent->deleteLater();m_agent=nullptr;fail(tr("Could not register the network authentication agent."));return;}}
 if(!m_agentRegistered){write(base+"/AgentManager",nm+".AgentManager","RegisterWithCapabilities",{QString("org.harbor.NetworkSettings"),uint(1)},[this,path](const QDBusMessage &){m_agentRegistered=true;activateWithAgent(path);});return;}
 m_activatingPath=path;
 write(base,nm,"ActivateConnection",{QVariant::fromValue(QDBusObjectPath(path)),QVariant::fromValue(QDBusObjectPath("/")),QVariant::fromValue(QDBusObjectPath("/"))});
 m_secretTimer.start(120000);
}
bool NetworkSecretAgent::trusted() const{return calledFromDBus()&&m_owner->m_bus.interface()&&message().service()==m_owner->m_bus.interface()->serviceOwner(nm).value();}
NetworkProfile NetworkSecretAgent::GetSecrets(const NetworkProfile &connection,const QDBusObjectPath &path,const QString &setting,const QStringList &hints,uint flags){
 if(!trusted()||path.path()!=m_owner->m_activatingPath||m_waiting||!(flags&1)||!QStringList{"vpn","802-11-wireless-security","802-1x"}.contains(setting)){if(calledFromDBus())sendErrorReply("org.freedesktop.NetworkManager.SecretAgent.Error.NoSecrets","No matching user-initiated authentication request.");return {};}
 m_keys.clear();for(const auto &hint:hints)if(QRegularExpression("^[A-Za-z0-9_.-]{1,64}$").match(hint).hasMatch()&&!hint.startsWith("x-vpn-"))m_keys<<hint;
 if(m_keys.isEmpty())m_keys<<(setting=="802-11-wireless-security"?"psk":setting=="802-1x"&&connection.value("802-1x").value("eap").toStringList().contains("tls")?"private-key-password":"password");
 QVariantList fields;for(const auto &key:m_keys)fields.append(QVariantMap{{"key",key},{"label",key},{"secret",!key.contains("username")&&!key.contains("identity")}});
 setDelayedReply(true);m_pending=message();m_waiting=true;m_setting=setting;
 m_owner->m_secretPrompt={{"name",connection.value("connection").value("id")},{"fields",fields}};emit m_owner->changed();return {};
}
void NetworkSecretAgent::respond(bool accept,const QVariantMap &values){
 if(!m_waiting)return;
 if(accept){for(const auto &key:m_keys)if(values.value(key).toString().isEmpty()){m_owner->fail(tr("Fill in each requested credential."));return;}
  QVariantMap credentials;for(const auto &key:m_keys)credentials[key]=values.value(key).toString();NetworkProfile result;
  if(m_setting=="vpn"){QMap<QString,QString> secrets;for(auto i=credentials.begin();i!=credentials.end();++i)secrets[i.key()]=i.value().toString();result["vpn"]={{"secrets",QVariant::fromValue(secrets)}};}else result[m_setting]=credentials;
  m_owner->m_bus.send(m_pending.createReply({QVariant::fromValue(result)}));
 }else m_owner->m_bus.send(m_pending.createErrorReply("org.freedesktop.NetworkManager.SecretAgent.Error.UserCanceled","Authentication cancelled by the user."));
 m_waiting=false;m_pending=QDBusMessage();m_keys.clear();m_owner->m_secretPrompt.clear();emit m_owner->changed();
}
void NetworkSecretAgent::CancelGetSecrets(const QDBusObjectPath &path,const QString &){if(trusted()&&path.path()==m_owner->m_activatingPath)respond(false,{});}
void NetworkSecretAgent::SaveSecrets(const NetworkProfile &,const QDBusObjectPath &){if(calledFromDBus())sendErrorReply("org.freedesktop.NetworkManager.SecretAgent.Error.NoSecrets","Harbor only supplies credentials for the current connection attempt.");}
void NetworkSecretAgent::DeleteSecrets(const NetworkProfile &,const QDBusObjectPath &){if(!trusted()&&calledFromDBus())sendErrorReply("org.freedesktop.NetworkManager.SecretAgent.Error.PermissionDenied","Unexpected caller.");}
