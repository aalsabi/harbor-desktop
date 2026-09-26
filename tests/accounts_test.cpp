#include <QtTest>
#include <QDBusVirtualObject>
#include <QDBusObjectPath>
#include <QDBusMetaType>
#include <QDBusMessage>
#include <unistd.h>
#include "core/Accounts.h"
class AccountsFake:public QDBusVirtualObject{
public:
 QMap<QString,QVariantMap> users;QStringList methods;QVariantList lastArguments;QString lastHash;bool reject=false,failPassword=false;
 AccountsFake(){reset();}
 void reset(){users.clear();methods.clear();lastHash.clear();reject=false;failPassword=false;add("root",0,1,true);add("me",getuid(),1);add("admin",65001,1);add("person",65002,0);}
 void add(QString name,qulonglong uid,int type,bool system=false){users["/org/freedesktop/Accounts/"+name]={{"UserName",name},{"RealName",name},{"Uid",QVariant::fromValue(uid)},{"AccountType",type},{"SystemAccount",system},{"LocalAccount",true},{"Locked",false}};}
 QString introspect(const QString&)const override{return {};}
 bool handleMessage(const QDBusMessage &m,const QDBusConnection &bus)override{
  if(m.member()=="ListCachedUsers"){QList<QDBusObjectPath> paths;for(const auto &path:users.keys())paths.append(QDBusObjectPath(path));bus.send(m.createReply({QVariant::fromValue(paths)}));return true;}
  if(m.member()=="GetAll"){bus.send(m.createReply(QVariantList{users.value(m.path())}));return true;}
  methods.append(m.member());lastArguments=m.arguments();if(reject||(failPassword&&m.member()=="SetPassword")){bus.send(m.createErrorReply("org.freedesktop.Accounts.Error.PermissionDenied","Fixture authorization denied"));return true;}
  if(m.member()=="CreateUser"){add(m.arguments()[0].toString(),65003,m.arguments()[2].toInt());bus.send(m.createReply({QVariant::fromValue(QDBusObjectPath("/org/freedesktop/Accounts/"+m.arguments()[0].toString()))}));return true;}
  if(m.member()=="SetPassword")lastHash=m.arguments()[0].toString();
  else if(m.member()=="SetRealName")users[m.path()]["RealName"]=m.arguments()[0];
  else if(m.member()=="SetAccountType")users[m.path()]["AccountType"]=m.arguments()[0];
  else if(m.member()=="DeleteUser"){for(auto i=users.begin();i!=users.end();++i)if(i.value()["Uid"].toULongLong()==m.arguments()[0].toULongLong()){users.erase(i);break;}}
  else {bus.send(m.createErrorReply("org.freedesktop.DBus.Error.UnknownMethod","Unexpected method"));return true;}
  bus.send(m.createReply());return true;
 }
};
class AccountsTest:public QObject{
 Q_OBJECT
 AccountsFake fake;
 QDBusConnection bus=QDBusConnection::sessionBus();
 const QString base="/org/freedesktop/Accounts/";
private slots:
 void initTestCase(){qDBusRegisterMetaType<QList<QDBusObjectPath>>();QVERIFY(bus.isConnected());QVERIFY(bus.registerService("org.freedesktop.Accounts"));QVERIFY(bus.registerVirtualObject("/",&fake,QDBusConnection::SubPath));}
 void init(){fake.reset();}
 void cleanupTestCase(){bus.unregisterObject("/");bus.unregisterService("org.freedesktop.Accounts");}
 void validation(){QVERIFY(Accounts::validUserName("new_user-2"));for(const auto &name:QStringList{"Root","a b","../a","-a","","a;id"})QVERIFY(!Accounts::validUserName(name));QVERIFY(Accounts::passwordHash("short").isEmpty());auto a=Accounts::passwordHash("A strong test password"),b=Accounts::passwordHash("A strong test password");QVERIFY(a.startsWith("$6$rounds=100000$"));QVERIFY(a!=b);QVERIFY(!a.contains("strong"));}
 void createAndPassword(){Accounts accounts(bus);accounts.createUser("new_user","New User",0,"A strong test password");QTRY_VERIFY(!accounts.busy());QCOMPARE(fake.methods,QStringList({"CreateUser","SetPassword"}));QVERIFY(fake.lastHash.startsWith("$6$"));QVERIFY(!fake.lastHash.contains("password"));QCOMPARE(accounts.users().size(),5);}
 void passwordFailureIsExplicit(){fake.failPassword=true;Accounts accounts(bus);accounts.createUser("new_user","New User",0,"A strong test password");QTRY_VERIFY(!accounts.busy());QVERIFY(accounts.error().contains("Account created"));QVERIFY(fake.users.contains(base+"new_user"));}
 void deletionRequiresConfirmationAndKeepsFiles(){Accounts accounts(bus);accounts.deleteUser(base+"person");QVERIFY(!accounts.error().isEmpty());QVERIFY(fake.methods.isEmpty());accounts.deleteUser(base+"person",false,true);QTRY_VERIFY(!accounts.busy());QCOMPARE(fake.methods,QStringList{"DeleteUser"});QCOMPARE(fake.lastArguments[0].toLongLong(),qlonglong(65002));QCOMPARE(fake.lastArguments[1].toBool(),false);}
 void protectedAccounts(){Accounts accounts(bus);for(const auto &name:QStringList{"root","me"}){accounts.deleteUser(base+name,true,true);QTRY_VERIFY(!accounts.busy());accounts.setAccountType(base+name,0,true);QTRY_VERIFY(!accounts.busy());}QVERIFY(fake.methods.isEmpty());QVERIFY(!accounts.error().isEmpty());}
 void lastAdminFreshGuard(){Accounts accounts(bus);accounts.refresh();QTRY_VERIFY(!accounts.busy());fake.users[base+"me"]["AccountType"]=0;accounts.setAccountType(base+"admin",0,true);QTRY_VERIFY(!accounts.busy());QVERIFY(fake.methods.isEmpty());QVERIFY(accounts.error().contains("administrator"));accounts.deleteUser(base+"admin",false,true);QTRY_VERIFY(!accounts.busy());QVERIFY(fake.methods.isEmpty());}
 void typeNameAndPassword(){Accounts accounts(bus);accounts.setAccountType(base+"person",1,true);QTRY_VERIFY(!accounts.busy());QCOMPARE(fake.users[base+"person"]["AccountType"].toInt(),1);accounts.setRealName(base+"person","Changed Name");QTRY_VERIFY(!accounts.busy());QCOMPARE(fake.users[base+"person"]["RealName"].toString(),QString("Changed Name"));accounts.setPassword(base+"person","A second test password");QTRY_VERIFY(!accounts.busy());QVERIFY(fake.lastHash.startsWith("$6$"));}
 void authorizationDenied(){fake.reject=true;Accounts accounts(bus);accounts.setAccountType(base+"person",1,true);QTRY_VERIFY(!accounts.busy());QVERIFY(accounts.error().contains("authorization denied"));QCOMPARE(fake.users[base+"person"]["AccountType"].toInt(),0);}
};
QTEST_GUILESS_MAIN(AccountsTest)
#include "accounts_test.moc"
