#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include "core/Applications.h"
class LaunchTest:public QObject{
 Q_OBJECT
 QTemporaryDir temp;
 QString log;
private slots:
 void initTestCase(){
  qputenv("XDG_DATA_HOME",temp.path().toUtf8());QDir().mkpath(temp.path()+"/applications");log=temp.path()+"/calls";
  QFile worker(temp.path()+"/app.py");QVERIFY(worker.open(QIODevice::WriteOnly));worker.write("import pathlib,sys,time,os,signal\nsignal.signal(signal.SIGPIPE,signal.SIG_DFL)\nwith pathlib.Path(sys.argv[1]).open('a') as f:f.write('started\\n')\ntime.sleep(.2)\nos.write(2,b'late application diagnostic\\n')\npathlib.Path(sys.argv[1]+'.alive').write_text('alive')\n");worker.close();
  QFile entry(temp.path()+"/applications/harbor-slow-test.desktop");QVERIFY(entry.open(QIODevice::WriteOnly));entry.write(("[Desktop Entry]\nType=Application\nName=Slow fixture\nExec=/usr/bin/python3 "+temp.path()+"/app.py "+log+"\nStartupWMClass=SlowFixture\n").toUtf8());
 }
 void failedLaunchCanBeRetried(){
  QTemporaryDir bin;QFile gio(bin.path()+"/gio");QVERIFY(gio.open(QIODevice::WriteOnly));gio.write("#!/bin/sh\necho fixture-launch-error >&2\nexit 3\n");gio.close();QVERIFY(gio.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
  auto old=qgetenv("PATH");qputenv("PATH",bin.path().toUtf8());Applications apps;QSignalSpy errors(&apps,&Applications::error);
  QVERIFY(apps.launch("harbor-slow-test.desktop"));QTRY_COMPARE(errors.size(),1);QVERIFY(apps.property("pendingLaunches").toStringList().isEmpty());
  QVERIFY(apps.launch("harbor-slow-test.desktop"));QTRY_COMPARE(errors.size(),2);qputenv("PATH",old);
 }
 void repeatedClicksLaunchOnceAndExistingWindowIsActivated(){
  Applications apps;for(int i=0;i<8;++i)QVERIFY(apps.launch("harbor-slow-test.desktop"));
  QTest::qWait(450);QFile calls(log);QVERIFY(calls.open(QIODevice::ReadOnly));QCOMPARE(calls.readAll(),QByteArray("started\n"));calls.close();QVERIFY(QFile::exists(log+".alive"));
  QVariantList windows{QVariantMap{{"id","window-1"},{"appId","SlowFixture"}}};
  QSignalSpy activated(&apps,SIGNAL(activateRequested(QString)));QVERIFY(activated.isValid());
  QVERIFY(QMetaObject::invokeMethod(&apps,"setWindows",Q_ARG(QVariantList,windows)));QCOMPARE(activated.size(),1);
  QVERIFY(apps.launch("harbor-slow-test.desktop"));QCOMPARE(activated.size(),2);QCOMPARE(activated.first().first().toString(),QString("window-1"));
  QVERIFY(calls.open(QIODevice::ReadOnly));QCOMPARE(calls.readAll(),QByteArray("started\n"));
 }
};
QTEST_GUILESS_MAIN(LaunchTest)
#include "launch_test.moc"
