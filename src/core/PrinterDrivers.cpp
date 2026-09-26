#include "PrinterDrivers.h"
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusObjectPath>
#include <QDBusArgument>
#include <QDBusServiceWatcher>
#include <QSet>
namespace {const QString service="org.freedesktop.PackageKit",root="/org/freedesktop/PackageKit",iface="org.freedesktop.PackageKit.Transaction";}
PrinterDrivers::PrinterDrivers(QObject *parent):PrinterDrivers(QDBusConnection::systemBus(),parent){}
PrinterDrivers::PrinterDrivers(const QDBusConnection &connection,QObject *parent):QObject(parent),bus(connection){
 m_watchdog.setSingleShot(true);m_watchdog.setInterval(1800000);connect(&m_watchdog,&QTimer::timeout,this,[this]{if(m_busy){m_status=tr("The package transaction is taking longer than expected. Its outcome is not yet known; do not start another package manager.");emit changed();}});
 auto watcher=new QDBusServiceWatcher(service,bus,QDBusServiceWatcher::WatchForUnregistration,this);connect(watcher,&QDBusServiceWatcher::serviceUnregistered,this,[this]{if(m_busy)fail(tr("The package service disconnected. The transaction outcome is unknown. Check again before retrying."));});
}
void PrinterDrivers::disconnectTransaction(){if(m_path.isEmpty())return;bus.disconnect(service,m_path,iface,QString(),this,SLOT(transactionSignal(QDBusMessage)));bus.disconnect(service,m_path,"org.freedesktop.DBus.Properties","PropertiesChanged",this,SLOT(propertiesSignal(QDBusMessage)));m_path.clear();m_watchdog.stop();}
void PrinterDrivers::fail(const QString &error){disconnectTransaction();m_busy=false;m_ready=false;m_error=error;m_status=tr("Driver installation failed");emit changed();}
QString PrinterDrivers::suggestedFamily(const QString &model)const{auto text=model.toLower();if(text.contains("hp")||text.contains("hewlett"))return "HP";if(text.contains("epson"))return "Epson";if(text.contains("brother"))return "Brother";if(text.contains("samsung"))return "Samsung";return "Other";}
void PrinterDrivers::search(const QString &family){if(m_busy)return;
 const QMap<QString,QStringList> choices{{"HP",{"printer-driver-hpcups","printer-driver-hpijs"}},{"Epson",{"printer-driver-escpr","printer-driver-gutenprint"}},{"Brother",{"printer-driver-brlaser"}},{"Samsung",{"printer-driver-splix"}},{"Other",{"printer-driver-gutenprint","printer-driver-foomatic-db"}}};
 m_packages.clear();m_preview.clear();m_selected.clear();m_ready=false;m_checked=false;if(!choices.contains(family)){fail(tr("Choose a supported printer family."));return;}m_names=choices.value(family);start(List);
}
void PrinterDrivers::discardPreview(){if(m_busy)return;m_ready=false;m_preview.clear();m_selected.clear();emit changed();}
void PrinterDrivers::prepare(const QStringList &ids){
 if(m_busy)return;m_ready=false;m_preview.clear();m_selected.clear();QSet<QString> allowed;for(const auto &entry:m_packages){auto row=entry.toMap();if(!row.value("blocked").toBool())allowed.insert(row.value("id").toString());}
 for(const auto &id:ids){if(!allowed.contains(id)){fail(tr("Choose printer drivers from the current list. Blocked packages cannot be installed."));return;}if(!m_selected.contains(id))m_selected.append(id);}
 if(m_selected.isEmpty()){fail(tr("Select at least one driver package to review."));return;}start(Preview);
}
void PrinterDrivers::install(bool confirmed){if(m_busy)return;if(!confirmed||!m_ready||m_selected.isEmpty()){m_error=tr("Review the dependency preview and explicitly confirm installation first.");emit changed();return;}m_ready=false;start(Install);}
void PrinterDrivers::start(Mode mode){
 disconnectTransaction();m_mode=mode;m_busy=true;m_percentage=-1;m_error.clear();m_status=mode==List?tr("Finding repository printer drivers…"):mode==Preview?tr("Preparing dependency preview…"):tr("Installing trusted printer drivers…");emit changed();
 auto msg=QDBusMessage::createMethodCall(service,root,service,"CreateTransaction");auto w=new QDBusPendingCallWatcher(bus.asyncCall(msg,15000),this);
 connect(w,&QDBusPendingCallWatcher::finished,this,[this,w]{QDBusPendingReply<QDBusObjectPath> reply=*w;w->deleteLater();if(reply.isError()){fail(tr("PackageKit is unavailable or this operation is unsupported: %1").arg(reply.error().message()));return;}m_path=reply.value().path();if(m_path.isEmpty()||m_path=="/"){fail(tr("The package service returned an invalid transaction."));return;}
 bus.connect(service,m_path,iface,QString(),this,SLOT(transactionSignal(QDBusMessage)));bus.connect(service,m_path,"org.freedesktop.DBus.Properties","PropertiesChanged",this,SLOT(propertiesSignal(QDBusMessage)));m_watchdog.start();
 auto hints=QDBusMessage::createMethodCall(service,m_path,iface,"SetHints");hints.setArguments({QStringList{"interactive=true","cache-age=2147483647"}});auto h=new QDBusPendingCallWatcher(bus.asyncCall(hints,15000),this);auto path=m_path;
 connect(h,&QDBusPendingCallWatcher::finished,this,[this,h,path]{auto reply=h->reply();h->deleteLater();if(path!=m_path)return;if(reply.type()==QDBusMessage::ErrorMessage){fail(reply.errorMessage());return;}
 if(m_mode==List)invoke(path,"Resolve",{QVariant::fromValue<qulonglong>(qulonglong(1)<<3),m_names});else invoke(path,"InstallPackages",{QVariant::fromValue<qulonglong>(OnlyTrusted|(m_mode==Preview?Simulate:0)),m_selected});
 });});
}
void PrinterDrivers::invoke(const QString &path,const QString &method,const QVariantList &args){auto msg=QDBusMessage::createMethodCall(service,path,iface,method);msg.setArguments(args);msg.setInteractiveAuthorizationAllowed(true);auto w=new QDBusPendingCallWatcher(bus.asyncCall(msg,120000),this);connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,path]{auto reply=w->reply();w->deleteLater();if(path==m_path&&reply.type()==QDBusMessage::ErrorMessage)fail(reply.errorMessage());});}
QVariantMap PrinterDrivers::package(uint info,const QString &id,const QString &summary)const{auto fields=id.split(';');QString action=info==13?tr("Remove"):info==15?tr("Replace"):info==12?tr("Install"):info==11?tr("Update"):tr("Change");return {{"id",id},{"name",fields.value(0)},{"version",fields.value(1)},{"summary",summary},{"info",info},{"blocked",info==9},{"security",info==8},{"action",action}};}
void PrinterDrivers::transactionSignal(const QDBusMessage &message){
 if(!m_busy||message.path()!=m_path)return;auto args=message.arguments();auto member=message.member();
 if(member=="Package"&&args.size()==3){uint info=args[0].toUInt();QString id=args[1].toString();if(id.split(';').size()!=4)return;if(m_mode==List&&(!m_names.contains(id.section(';',0,0))||info!=2))return;auto row=package(info,id,args[2].toString());auto &target=m_mode==List?m_packages:m_preview;if(m_mode==List||m_mode==Preview){bool found=false;for(auto &entry:target)if(entry.toMap().value("id").toString()==id){entry=row;found=true;}if(!found)target.append(row);}else if(m_mode==Install)m_status=tr("Installing %1…").arg(row.value("name").toString());emit changed();}
 else if(member=="ErrorCode"&&args.size()==2){m_error=tr("Package service error %1: %2").arg(args[0].toUInt()).arg(args[1].toString());emit changed();}
 else if(member=="EulaRequired"||member=="RepoSignatureRequired"||member=="MediaChangeRequired"){m_error=tr("This installation requires a license agreement, repository trust decision, or installation media. Harbor has not accepted it. Resolve it in your distribution's package tools, then check again.");m_ready=false;emit changed();}
 else if(member=="RequireRestart"&&args.size()==2){uint type=args[0].toUInt();if(m_mode==Install&&type>1){if(type==4||type==6)m_restart=tr("Restart the computer to finish applying printer drivers.");else if(m_restart.isEmpty())m_restart=type==3||type==5?tr("Sign out and back in to finish applying printer drivers."):tr("Restart updated applications to use their new versions.");emit changed();}}
 else if(member=="Finished"&&args.size()==2)finish(args[0].toUInt());
}
void PrinterDrivers::propertiesSignal(const QDBusMessage &message){if(!m_busy||message.path()!=m_path||message.arguments().size()<2)return;if(message.arguments()[0].toString()!=iface)return;auto changed=qdbus_cast<QVariantMap>(message.arguments()[1]);if(changed.contains("Percentage")){uint value=changed.value("Percentage").toUInt();m_percentage=value<=100?int(value):-1;}
 if(changed.contains("Status")){switch(changed.value("Status").toUInt()){
 case 1:m_status=tr("Waiting for the package service…");break;
 case 6:m_status=tr("Removing packages…");break;
 case 7:m_status=tr("Refreshing package information…");break;
 case 8:case 20:case 21:case 22:case 23:case 24:case 25:m_status=tr("Downloading package data…");break;
 case 9:case 10:case 16:m_status=m_mode==Preview?tr("Simulating package changes…"):tr("Installing driver packages…");break;
 case 11:m_status=tr("Cleaning up…");break;
 case 13:m_status=tr("Resolving dependencies…");break;
 case 14:m_status=tr("Verifying package signatures…");break;
 case 30:m_status=tr("Waiting for another package manager to finish…");break;
 case 31:m_status=tr("Waiting for system authorization…");break;
 default:break;}}
 emit this->changed();}
void PrinterDrivers::finish(uint exit){auto mode=m_mode;disconnectTransaction();m_busy=false;if(exit!=1||!m_error.isEmpty()){m_ready=false;if(m_error.isEmpty())m_error=tr("The package transaction did not complete (result %1).").arg(exit);m_status=tr("Driver installation did not complete");emit changed();return;}
 if(mode==List){m_checked=true;m_status=m_packages.isEmpty()?tr("No missing driver packages were found in the enabled repositories. These drivers may already be installed, or your model may need a different package."):tr("Available printer driver packages loaded.");}
 if(mode==Preview){if(m_preview.isEmpty()){fail(tr("The backend did not provide a dependency preview. Installation has not started."));return;}m_ready=true;m_status=tr("Review all package changes, then confirm installation.");}
 if(mode==Install){m_ready=false;m_selected.clear();m_packages.clear();m_checked=false;m_status=tr("Printer drivers installed. Select your printer again to load the new drivers.");}m_percentage=100;emit changed();if(mode==Install)emit installed();
}
