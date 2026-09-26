#include <QtTest>
#include <QTemporaryDir>
#include <QDBusConnection>
#include <QDBusContext>
#include <QSettings>
#include "core/AccessibilitySettings.h"
class EffectsMock:public QObject{
 Q_OBJECT
 Q_CLASSINFO("D-Bus Interface","org.kde.kwin.Effects")
 Q_PROPERTY(QStringList loadedEffects MEMBER loaded)
 Q_PROPERTY(QStringList listOfEffects MEMBER available)
 Q_PROPERTY(QStringList activeEffects MEMBER active)
public:QStringList loaded,active,available{"zoom","invert"};QStringList calls;
public slots:bool loadEffect(QString n){calls<<n;if(!loaded.contains(n))loaded<<n;return true;}void unloadEffect(QString n){calls<<"off:"+n;loaded.removeAll(n);active.removeAll(n);}
};
class ShortcutMock:public QObject,protected QDBusContext{
 Q_OBJECT
 Q_CLASSINFO("D-Bus Interface","org.kde.kglobalaccel.Component")
public:EffectsMock *effects=nullptr;QStringList calls;bool reject=false,noEffect=false;
public slots:void invokeShortcut(QString name){calls<<name;if(reject){sendErrorReply("org.kde.kglobalaccel.Error.Failed","Fixture shortcut denied");return;}if(name=="Invert"&&!noEffect){if(effects->active.contains("invert"))effects->active.removeAll("invert");else effects->active<<"invert";}}
};
class AccessibilityTest:public QObject{
 Q_OBJECT
 QDBusConnection bus=QDBusConnection::sessionBus();EffectsMock effects;ShortcutMock shortcuts;
private slots:
 void initTestCase(){QVERIFY(bus.registerService("org.kde.KWin"));QVERIFY(bus.registerService("org.kde.kglobalaccel"));QVERIFY(bus.registerObject("/Effects",&effects,QDBusConnection::ExportAllProperties|QDBusConnection::ExportAllSlots));shortcuts.effects=&effects;QVERIFY(bus.registerObject("/component/kwin",&shortcuts,QDBusConnection::ExportAllSlots));}
 void init(){effects.loaded.clear();effects.active.clear();effects.calls.clear();shortcuts.calls.clear();shortcuts.reject=false;shortcuts.noEffect=false;}
 void cleanupTestCase(){bus.unregisterObject("/Effects");bus.unregisterObject("/component/kwin");bus.unregisterService("org.kde.KWin");bus.unregisterService("org.kde.kglobalaccel");}
 void supportedActions(){QTemporaryDir temp;AccessibilitySettings s(nullptr,temp.filePath("prefs.ini"),bus);s.refresh();QTRY_VERIFY(!s.busy());QVERIFY(s.state()["effectsAvailable"].toBool());s.setEffect("zoom",true);QTRY_VERIFY(!s.busy());QVERIFY(effects.loaded.contains("zoom"));s.setEffect("untrusted",true);QCOMPARE(effects.calls.size(),1);s.setEffect("zoom",false);QTRY_VERIFY(!s.busy());QVERIFY(!effects.loaded.contains("zoom"));}
 void invertReflectsActualState(){QTemporaryDir temp;effects.loaded<<"invert";AccessibilitySettings s(nullptr,temp.filePath("prefs.ini"),bus);s.refresh();QTRY_VERIFY(!s.busy());QVERIFY(!s.state()["active"].toStringList().contains("invert"));s.setEffect("invert",true);QTRY_VERIFY(!s.busy());QVERIFY(s.state()["active"].toStringList().contains("invert"));QCOMPARE(shortcuts.calls,QStringList{"Invert"});s.setEffect("invert",true);QTRY_VERIFY(!s.busy());QCOMPARE(shortcuts.calls.size(),1);s.setEffect("invert",false);QTRY_VERIFY(!s.busy());QVERIFY(!s.state()["active"].toStringList().contains("invert"));QVERIFY(!effects.loaded.contains("invert"));}
 void shortcutFailureNotSaved(){QTemporaryDir temp;auto path=temp.filePath("prefs.ini");shortcuts.reject=true;AccessibilitySettings s(nullptr,path,bus);s.setEffect("invert",true);QTRY_VERIFY(!s.busy());QVERIFY(s.error().contains("shortcut denied"));QVERIFY(!s.state()["active"].toStringList().contains("invert"));QSettings saved(path,QSettings::IniFormat);QVERIFY(!saved.value("effects/invert",false).toBool());}
 void successfulShortcutMustActuallyActivate(){QTemporaryDir temp;shortcuts.noEffect=true;AccessibilitySettings s(nullptr,temp.filePath("prefs.ini"),bus);s.setEffect("invert",true);QTRY_VERIFY(!s.busy());QVERIFY(s.error().contains("did not apply"));QVERIFY(!s.state()["active"].toStringList().contains("invert"));}
 void restoreActivatesAndIsIdempotent(){QTemporaryDir temp;auto path=temp.filePath("prefs.ini");{QSettings settings(path,QSettings::IniFormat);settings.setValue("effects/invert",true);settings.sync();}AccessibilitySettings s(nullptr,path,bus);s.restore();QTRY_VERIFY(s.state()["active"].toStringList().contains("invert"));QTRY_VERIFY(!s.busy());QCOMPARE(shortcuts.calls.size(),1);s.restore();QTRY_VERIFY(!s.busy());QCOMPARE(shortcuts.calls.size(),1);QVERIFY(effects.active.contains("invert"));}
};
QTEST_GUILESS_MAIN(AccessibilityTest)
#include "accessibility_test.moc"
