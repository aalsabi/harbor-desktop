#include <QtTest>
#include <QDBusVirtualObject>
#include <QDBusMessage>
#include <QDBusConnectionInterface>
#include <QDBusVariant>
#include "core/PointerSettings.h"

class PointerFake:public QDBusVirtualObject {
public:
 QMap<QString,QVariantMap> devices{
  {"event1",{{"name","Mouse"},{"pointer",true},{"touchpad",false},{"supportsNaturalScroll",true},{"naturalScroll",false},{"supportsPointerAcceleration",true},{"pointerAcceleration",0.0},{"supportsLeftHanded",false},{"leftHanded",false}}},
  {"event2",{{"name","Touchpad"},{"pointer",true},{"touchpad",true},{"tapFingerCount",3},{"tapToClick",false},{"tapAndDrag",false},{"supportsDisableWhileTyping",true},{"disableWhileTyping",false}}}
 };
 int mutations=0;
 bool reject=false,ignore=false,failReads=false;
 QString introspect(const QString &)const override{return {};}
 bool handleMessage(const QDBusMessage &m,const QDBusConnection &bus)override {
  if(m.member()=="Get") {bus.send(m.createReply({QVariant::fromValue(QDBusVariant(devices.keys()))}));return true;}
  auto name=m.path().section('/',-1);
  if(!devices.contains(name)){bus.send(m.createErrorReply("org.freedesktop.DBus.Error.UnknownObject","Device unplugged"));return true;}
  if(m.member()=="GetAll"&&failReads){bus.send(m.createErrorReply("org.freedesktop.DBus.Error.Failed","Cannot read device"));return true;}
  if(m.member()=="GetAll"){bus.send(m.createReply(QVariantList{devices.value(name)}));return true;}
  if(m.member()=="Set") {
   ++mutations;
   if(reject){bus.send(m.createErrorReply("org.freedesktop.DBus.Error.Failed","Rejected change"));return true;}
   if(!ignore)devices[name][m.arguments()[1].toString()]=qvariant_cast<QDBusVariant>(m.arguments()[2]).variant();
   bus.send(m.createReply());return true;
  }
  return false;
 }
};
class PointerSettingsTest:public QObject {
 Q_OBJECT
private slots:
 void privateBusRoundtrip() {
  auto bus=QDBusConnection::sessionBus();
  // CTest must run this executable under dbus-run-session; never claim org.kde.KWin.
  // Refuse the test if this is a live graphical session's compositor bus.
  QVERIFY(!bus.interface()->isServiceRegistered("org.kde.KWin").value());
  PointerFake fake;QVERIFY(bus.registerService("org.kde.KWin"));
  QVERIFY(bus.registerVirtualObject("/org/kde/KWin/InputDevice",&fake,QDBusConnection::SubPath));
  {
   PointerSettings settings(nullptr,bus);QVERIFY(settings.devices().isEmpty());settings.refresh();
   QTRY_VERIFY(!settings.busy());QVERIFY(settings.available());QCOMPARE(settings.devices().size(),2);
   QVERIFY(settings.devices()[0].toMap()["can_naturalScroll"].toBool());
   QVERIFY(!settings.devices()[0].toMap()["can_leftHanded"].toBool());
   settings.setSetting("event1","naturalScroll",true);QTRY_VERIFY(!settings.busy());
   QTRY_VERIFY(settings.devices()[0].toMap()["naturalScroll"].toBool());QVERIFY(settings.error().isEmpty());
   settings.setSetting("event2","tapToClick",true);QTRY_VERIFY(!settings.busy());QTRY_VERIFY(settings.devices()[1].toMap()["tapToClick"].toBool());
   settings.setSetting("event1","pointerAcceleration",0.45);QTRY_VERIFY(!settings.busy());QTRY_COMPARE(settings.devices()[0].toMap()["pointerAcceleration"].toDouble(),0.45);
   int count=fake.mutations;
   settings.setSetting("event1","enabled",false);settings.setSetting("event1","leftHanded",true);settings.setSetting("bad/path","naturalScroll",true);settings.setSetting("event1","pointerAcceleration",5);settings.setSetting("event1","naturalScroll","false");
   QCOMPARE(fake.mutations,count);
   fake.ignore=true;settings.setSetting("event1","naturalScroll",false);QTRY_VERIFY(!settings.busy());QVERIFY(settings.error().contains("did not accept"));QVERIFY(settings.devices()[0].toMap()["naturalScroll"].toBool());fake.ignore=false;
   fake.reject=true;settings.setSetting("event1","naturalScroll",false);QTRY_VERIFY(!settings.busy());QVERIFY(settings.error().contains("Rejected change"));fake.reject=false;
   settings.setSetting("event1","naturalScroll",true);QTRY_VERIFY(!settings.busy());QVERIFY(settings.error().isEmpty());
   fake.failReads=true;settings.refresh();QTRY_VERIFY(!settings.busy());QVERIFY(settings.error().contains("Cannot read device"));
   fake.failReads=false;settings.refresh();QTRY_VERIFY(!settings.busy());QCOMPARE(settings.devices().size(),2);QVERIFY(settings.error().isEmpty());
   fake.devices.remove("event2");settings.refresh();QTRY_COMPARE(settings.devices().size(),1);
   bus.unregisterService("org.kde.KWin");settings.refresh();QTRY_VERIFY(!settings.available());QVERIFY(settings.devices().isEmpty());
   QVERIFY(bus.registerService("org.kde.KWin"));settings.refresh();QTRY_VERIFY(settings.available());QTRY_VERIFY(!settings.busy());QVERIFY(settings.error().isEmpty());bus.unregisterService("org.kde.KWin");
  }
  bus.unregisterObject("/org/kde/KWin/InputDevice");
 }
};
QTEST_GUILESS_MAIN(PointerSettingsTest)
#include "pointer_settings_test.moc"
