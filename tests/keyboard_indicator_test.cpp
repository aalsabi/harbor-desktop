#include <QtTest>
#include <QDBusConnection>
#include <QDBusArgument>
#include <QDBusMetaType>
#include "core/Keyboard.h"
struct InputLayout {QString code,variant,name;};
Q_DECLARE_METATYPE(InputLayout)
Q_DECLARE_METATYPE(QList<InputLayout>)
QDBusArgument& operator<<(QDBusArgument& a,const InputLayout& v){a.beginStructure();a<<v.code<<v.variant<<v.name;a.endStructure();return a;}
const QDBusArgument& operator>>(const QDBusArgument& a,InputLayout& v){a.beginStructure();a>>v.code>>v.variant>>v.name;a.endStructure();return a;}
class LayoutFixture:public QObject {
 Q_OBJECT
 Q_CLASSINFO("D-Bus Interface","org.kde.KeyboardLayouts")
public:uint current=0;QList<InputLayout> layouts{{"us","","English (US)"},{"ara","","Arabic"}};
public slots:
 QList<InputLayout> getLayoutsList(){return layouts;}
 uint getLayout(){return current;}
 void switchToNextLayout(){current=(current+1)%layouts.size();emit layoutChanged(current);}
signals:void layoutChanged(uint index);void layoutListChanged();
};
class IndicatorTest:public QObject {
 Q_OBJECT
private slots:
 void followsCompositorInsteadOfSavedPreferences(){
  qDBusRegisterMetaType<InputLayout>();qDBusRegisterMetaType<QList<InputLayout>>();
  auto bus=QDBusConnection::sessionBus();LayoutFixture fixture;
  QVERIFY(bus.registerService("org.kde.keyboard"));QVERIFY(bus.registerObject("/Layouts",&fixture,QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllSignals));
  Keyboard keyboard;QTRY_COMPARE_WITH_TIMEOUT(keyboard.property("activeLabel").toString(),QString("EN"),1500);
  fixture.current=1;emit fixture.layoutChanged(1);
  QTRY_COMPARE_WITH_TIMEOUT(keyboard.property("activeLabel").toString(),QString::fromUtf8("ع"),1500);
  QCOMPARE(keyboard.property("activeName").toString(),QString("Arabic"));
  keyboard.switchNext();QTRY_COMPARE_WITH_TIMEOUT(keyboard.property("activeLabel").toString(),QString("EN"),1500);
  fixture.layouts={{"de","","German"}};fixture.current=0;emit fixture.layoutListChanged();
  QTRY_COMPARE_WITH_TIMEOUT(keyboard.property("activeLabel").toString(),QString("DE"),1500);
  bus.unregisterService("org.kde.keyboard");bus.unregisterObject("/Layouts");
  QMetaObject::invokeMethod(&keyboard,"refreshActive");QTRY_VERIFY(!keyboard.property("available").toBool());
  QCOMPARE(keyboard.property("activeLabel").toString(),QString::fromUtf8("⌨"));
 }
};
QTEST_GUILESS_MAIN(IndicatorTest)
#include "keyboard_indicator_test.moc"
