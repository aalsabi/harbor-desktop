#include "Accounts.h"
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusObjectPath>
#include <QDBusMessage>
#include <QRegularExpression>
#include <QLibrary>
#include <QRandomGenerator>
#include <memory>
#include <unistd.h>
namespace {const QString service="org.freedesktop.Accounts",root="/org/freedesktop/Accounts",iface="org.freedesktop.Accounts.User";
void wipe(QByteArray &bytes){volatile char *data=bytes.data();for(qsizetype i=0;i<bytes.size();++i)data[i]=0;bytes.clear();}
}
Accounts::Accounts(QObject *parent):Accounts(QDBusConnection::systemBus(),parent){}
Accounts::Accounts(const QDBusConnection &connection,QObject *parent):QObject(parent),bus(connection){}
bool Accounts::validUserName(const QString &name){return QRegularExpression("^[a-z_][a-z0-9_-]{0,31}$").match(name).hasMatch();}
QString Accounts::passwordHash(QString password){
 if(password.size()<8||password.size()>1024||password.contains(QChar(0)))return {};
 QLibrary crypt("crypt",1);using Crypt=char*(*)(const char*,const char*,void*,int);auto hash=reinterpret_cast<Crypt>(crypt.resolve("crypt_rn"));if(!hash)return {};
 QByteArray salt="$6$rounds=100000$";const QByteArray alphabet="./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";for(int i=0;i<16;++i)salt+=alphabet.at(QRandomGenerator::system()->bounded(alphabet.size()));salt+='$';
 QByteArray secret=password.toUtf8();password.fill(QChar(0));QByteArray state(131072,0);char *value=hash(secret.constData(),salt.constData(),state.data(),int(state.size()));QString result=value&&value[0]=='$'?QString::fromLatin1(value):QString();wipe(secret);wipe(state);return result;
}
void Accounts::fail(QString text){working=false;problem=text;emit changed();}
void Accounts::call(const QString &path,const QString &interface,const QString &method,const QVariantList &args,std::function<void(const QDBusMessage &)> done){
 auto msg=QDBusMessage::createMethodCall(service,path,interface,method);msg.setArguments(args);msg.setInteractiveAuthorizationAllowed(true);
 auto w=new QDBusPendingCallWatcher(bus.asyncCall(msg,120000),this);connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,done]{auto reply=w->reply();w->deleteLater();if(reply.type()==QDBusMessage::ErrorMessage){fail(reply.errorMessage());return;}if(done)done(reply);else load();});
}
void Accounts::load(std::function<void()> after){
 working=true;emit changed();auto msg=QDBusMessage::createMethodCall(service,root,service,"ListCachedUsers");
 auto w=new QDBusPendingCallWatcher(bus.asyncCall(msg,5000),this);connect(w,&QDBusPendingCallWatcher::finished,this,[this,w,after]{QDBusPendingReply<QList<QDBusObjectPath>> reply=*w;w->deleteLater();if(reply.isError()){rows.clear();fail(reply.error().message());return;}
 auto collected=std::make_shared<QVariantList>();auto left=std::make_shared<int>(reply.value().size());auto errors=std::make_shared<QString>();
 if(!*left){rows.clear();working=false;emit changed();if(after)after();return;}
 for(const auto &path:reply.value()){
 auto m=QDBusMessage::createMethodCall(service,path.path(),"org.freedesktop.DBus.Properties","GetAll");m<<iface;
 auto q=new QDBusPendingCallWatcher(bus.asyncCall(m,5000),this);connect(q,&QDBusPendingCallWatcher::finished,this,[this,q,path,after,collected,left,errors]{QDBusPendingReply<QVariantMap> result=*q;q->deleteLater();if(result.isError())*errors=result.error().message();else{auto row=result.value();row["path"]=path.path();collected->append(row);}
 if(--*left)return;if(!errors->isEmpty()){rows.clear();fail(*errors);return;}rows=*collected;working=false;for(auto &entry:rows){auto row=entry.toMap();row["canRemove"]=guard(row,true).isEmpty();row["isCurrent"]=row.value("Uid").toULongLong()==getuid();entry=row;}emit changed();if(after)after();});}
 });
}
void Accounts::refresh(){if(working)return;problem.clear();load();}
QVariantMap Accounts::user(const QString &path)const{for(const auto &entry:rows)if(entry.toMap().value("path").toString()==path)return entry.toMap();return {};}
QString Accounts::guard(const QVariantMap &row,bool destructive)const{
 if(row.isEmpty()||!row.contains("Uid"))return tr("Choose an account from the refreshed list.");
 auto uid=row.value("Uid").toULongLong();if(uid==0||row.value("SystemAccount",true).toBool()||!row.value("LocalAccount",false).toBool())return tr("System and remote accounts cannot be changed here.");
 if(destructive){if(uid==getuid())return tr("You cannot delete or demote your own account.");if(row.value("AccountType").toInt()==1){int count=0;for(const auto &entry:rows){auto r=entry.toMap();if(r.value("AccountType").toInt()==1&&!r.value("Locked").toBool()&&r.value("LocalAccount").toBool()&&!r.value("SystemAccount").toBool())++count;}if(count<=1&&!row.value("Locked").toBool())return tr("Keep at least one unlocked administrator account.");}}
 return {};
}
void Accounts::setRealName(QString path,QString name){if(working)return;name=name.trimmed();if(name.isEmpty()||name.size()>200||name.contains(QChar(0))){fail(tr("Enter a display name of 1–200 characters."));return;}problem.clear();load([this,path,name]{auto error=guard(user(path),false);if(!error.isEmpty()){fail(error);return;}working=true;emit changed();call(path,iface,"SetRealName",{name});});}
void Accounts::createUser(QString username,QString realName,int accountType,QString password){
 if(working)return;if(!validUserName(username)||realName.trimmed().isEmpty()||realName.size()>200||(accountType!=0&&accountType!=1)){fail(tr("Use a lowercase account name, a display name, and a valid account type."));return;}
 auto hash=passwordHash(password);password.fill(QChar(0));if(hash.isEmpty()){fail(tr("Use a password of 8–1024 characters. Password hashing also requires libcrypt1."));return;}
 working=true;problem.clear();emit changed();call(root,service,"CreateUser",{username,realName.trimmed(),accountType},[this,hash](const QDBusMessage &reply){QDBusPendingReply<QDBusObjectPath> result(reply);if(result.isError()||result.value().path().isEmpty()){fail(tr("The account service returned an invalid account."));return;}
 auto path=result.value().path();auto msg=QDBusMessage::createMethodCall(service,path,iface,"SetPassword");msg.setArguments({hash,QString()});msg.setInteractiveAuthorizationAllowed(true);auto w=new QDBusPendingCallWatcher(bus.asyncCall(msg,120000),this);connect(w,&QDBusPendingCallWatcher::finished,this,[this,w]{auto r=w->reply();w->deleteLater();if(r.type()==QDBusMessage::ErrorMessage)problem=tr("Account created, but its password could not be set. Set a password before signing in: %1").arg(r.errorMessage());load();});});
}
void Accounts::deleteUser(QString path,bool removeFiles,bool confirmed){if(working)return;if(!confirmed){fail(tr("Confirm account deletion first."));return;}problem.clear();load([this,path,removeFiles]{auto row=user(path);auto error=guard(row,true);if(!error.isEmpty()){fail(error);return;}working=true;emit changed();call(root,service,"DeleteUser",{QVariant::fromValue<qlonglong>(row.value("Uid").toLongLong()),removeFiles});});}
void Accounts::setAccountType(QString path,int type,bool confirmed){if(working)return;if((type!=0&&type!=1)||!confirmed){fail(tr("Confirm the account type change first."));return;}problem.clear();load([this,path,type]{auto row=user(path);auto error=guard(row,type==0);if(!error.isEmpty()){fail(error);return;}working=true;emit changed();call(path,iface,"SetAccountType",{type});});}
void Accounts::setPassword(QString path,QString password){if(working)return;auto hash=passwordHash(password);password.fill(QChar(0));if(hash.isEmpty()){fail(tr("Use a password of 8–1024 characters. Password hashing also requires libcrypt1."));return;}problem.clear();load([this,path,hash]{auto error=guard(user(path),false);if(!error.isEmpty()){fail(error);return;}working=true;emit changed();call(path,iface,"SetPassword",{hash,QString()});});}
