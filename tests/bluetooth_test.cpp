#include <QtTest>
#include <QDBusVirtualObject>
#include <QDBusMessage>
#include <QDBusVariant>
#include "core/Bluetooth.h"
class BluezFake:public QDBusVirtualObject {
public:
 BluezObjects objects{{QDBusObjectPath("/org/bluez/hci0"),{{"org.bluez.Adapter1",{{"Powered",false},{"Discovering",false}}}}},{QDBusObjectPath("/org/bluez/hci0/dev_A"),{{"org.bluez.Device1",{{"Address","AA:BB:CC:DD:EE:FF"},{"Alias","Headphones"},{"Connected",false},{"Paired",true}}}}}};
 int mutations=0;bool reject=false;
 QString introspect(const QString&)const override{return {};}
 bool handleMessage(const QDBusMessage& m,const QDBusConnection& bus) override {
  if(m.member()=="GetManagedObjects"){bus.send(m.createReply({QVariant::fromValue(objects)}));return true;}
  if(reject){bus.send(m.createErrorReply("org.bluez.Error.Failed","Test radio failure"));return true;}
  auto path=QDBusObjectPath(m.path());auto member=m.member();
  if(member=="Set"&&m.arguments().value(0).toString()=="org.bluez.Adapter1"&&m.arguments().value(1).toString()=="Powered")objects[path]["org.bluez.Adapter1"]["Powered"]=qvariant_cast<QDBusVariant>(m.arguments()[2]).variant();
  else if(member=="Connect"||member=="Disconnect")objects[path]["org.bluez.Device1"]["Connected"]=member=="Connect";
  else if(member=="StartDiscovery"||member=="StopDiscovery")objects[path]["org.bluez.Adapter1"]["Discovering"]=member=="StartDiscovery";
  else {bus.send(m.createErrorReply("org.bluez.Error.NotSupported","Unexpected method"));return true;}
  ++mutations;bus.send(m.createReply());return true;
 }
};
class BluetoothTest:public QObject {
 Q_OBJECT
private slots:
 void statesAndDirectActions(){
  auto bus=QDBusConnection::sessionBus();BluezFake fake;QVERIFY(bus.registerService("org.bluez"));QVERIFY(bus.registerVirtualObject("/",&fake,QDBusConnection::SubPath));
  {Bluetooth bluetooth(nullptr,bus);QTRY_VERIFY(bluetooth.state()["bluetoothAvailable"].toBool());QCOMPARE(bluetooth.state()["bluetoothStatus"].toString(),QString("Off"));
   fake.objects[QDBusObjectPath("/org/bluez/hci1")]["org.bluez.Adapter1"]={{"Powered",false},{"Discovering",false}};bluetooth.refresh();QTest::qWait(30);
   bluetooth.action("bluetooth",true);QTRY_COMPARE(bluetooth.state()["bluetoothStatus"].toString(),QString("On"));
   QVERIFY(fake.objects[QDBusObjectPath("/org/bluez/hci1")]["org.bluez.Adapter1"]["Powered"].toBool());
   bluetooth.action("bluetooth-connect","AA:BB:CC:DD:EE:FF");QTRY_COMPARE(bluetooth.state()["bluetoothStatus"].toString(),QString("Connected"));
   bluetooth.action("bluetooth-disconnect","AA:BB:CC:DD:EE:FF");QTRY_COMPARE(bluetooth.state()["bluetoothStatus"].toString(),QString("On"));
   auto calls=fake.mutations;bluetooth.action("bluetooth-connect","untrusted address");QTest::qWait(30);QCOMPARE(fake.mutations,calls);
   bluetooth.action("bluetooth-scan");QTRY_VERIFY(bluetooth.state()["bluetoothDiscovering"].toBool());bluetooth.action("bluetooth-stop-scan");QTRY_VERIFY(!fake.objects[QDBusObjectPath("/org/bluez/hci0")]["org.bluez.Adapter1"]["Discovering"].toBool());
   fake.reject=true;bluetooth.action("bluetooth",false);QTRY_VERIFY(bluetooth.state()["bluetoothError"].toString().contains("Test radio failure"));QVERIFY(bluetooth.state()["bluetoothPowered"].toBool());
   fake.reject=false;
   for(auto i=fake.objects.begin();i!=fake.objects.end();++i)if(i.value().contains("org.bluez.Adapter1"))i.value()["org.bluez.Adapter1"]["Powered"]=false;
   auto signal=QDBusMessage::createSignal("/org/bluez/hci0","org.freedesktop.DBus.Properties","PropertiesChanged");signal.setArguments({QString("org.bluez.Adapter1"),QVariantMap{{"Powered",false}},QStringList{}});QVERIFY(bus.send(signal));QTRY_COMPARE(bluetooth.state()["bluetoothStatus"].toString(),QString("Off"));
   fake.objects.clear();bluetooth.refresh();QTRY_VERIFY(!bluetooth.state()["bluetoothAvailable"].toBool());QVERIFY(bluetooth.state()["bluetoothDevices"].toList().isEmpty());
  }
  bus.unregisterObject("/");bus.unregisterService("org.bluez");
 }
};
QTEST_GUILESS_MAIN(BluetoothTest)
#include "bluetooth_test.moc"
