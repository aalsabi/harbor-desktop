#include <QtTest>
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QTemporaryDir>
#include <QFile>
#include "core/Keyboard.h"
class KeyboardNotifyTest:public QObject {
 Q_OBJECT
 int legacyCount=0,modernCount=0;
 QHash<QString,QByteArrayList> changes;
public slots:
 void legacy(){++legacyCount;}
 void modern(QHash<QString,QByteArrayList> value){++modernCount;changes=value;}
private slots:
 void applyNotifiesOldAndNewKWin(){
  qDBusRegisterMetaType<QByteArrayList>();qDBusRegisterMetaType<QHash<QString,QByteArrayList>>();
  auto bus=QDBusConnection::sessionBus();
  QVERIFY(bus.connect({},"/Layouts","org.kde.keyboard","reloadConfig",this,SLOT(legacy())));
  QVERIFY(bus.connect({},"/kxkbrc","org.kde.kconfig.notify","ConfigChanged",this,SLOT(modern(QHash<QString,QByteArrayList>))));
  QTemporaryDir temp;QFile helper(temp.filePath("harbor-keyboard"));QVERIFY(helper.open(QIODevice::WriteOnly));helper.write("#!/bin/sh\necho '{\"layouts\":[\"us\",\"ara\"],\"variants\":[\"\",\"\"],\"shortcut\":\"grp:alt_shift_toggle\"}'\n");helper.close();QVERIFY(helper.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
  const auto oldPath=qgetenv("PATH"),oldDesktop=qgetenv("XDG_SESSION_DESKTOP");
  qputenv("PATH",temp.path().toUtf8());qputenv("XDG_SESSION_DESKTOP","harbor");
  Keyboard keyboard;QTRY_VERIFY(!keyboard.state().isEmpty());QCOMPARE(legacyCount,0);QCOMPARE(modernCount,0);
  keyboard.apply({"us","ara"},"grp:alt_shift_toggle");QTRY_VERIFY(!keyboard.busy());
  qputenv("PATH",oldPath);qputenv("XDG_SESSION_DESKTOP",oldDesktop);
  QTRY_COMPARE(legacyCount,1);QTRY_COMPARE_WITH_TIMEOUT(modernCount,1,1500);
  QVERIFY(changes.value("Layout").contains("LayoutList"));QVERIFY(changes.value("Layout").contains("Options"));
 }
};
QTEST_GUILESS_MAIN(KeyboardNotifyTest)
#include "keyboard_notify_test.moc"
