#include <QtTest>
#include <QTemporaryDir>
#include "core/Preferences.h"
#include "core/Command.h"
class CoreTest:public QObject {
 Q_OBJECT
private slots:
 void invalidPreferencesDoNotPersist(){QTemporaryDir d; Preferences p(d.filePath("settings.ini")); p.setOpacity(-3); QCOMPARE(p.opacity(),0.9); p.setOpacity(0.7); Preferences loaded(d.filePath("settings.ini")); QCOMPARE(loaded.opacity(),0.7);}
 void externalPreferencesNotify(){QTemporaryDir d; Preferences p(d.filePath("settings.ini")); QSignalSpy change(&p,&Preferences::changed); QProcess writer; writer.start("/usr/bin/python3",{"-c","import sys;open(sys.argv[1],'w').write(chr(10).join(['[General]','dark=false','language=ar','']))",d.filePath("settings.ini")}); QVERIFY(writer.waitForFinished()); QCOMPARE(writer.exitCode(),0); QTRY_VERIFY_WITH_TIMEOUT(change.count()>0,3000); QCOMPARE(p.dark(),false); QCOMPARE(p.language(),QString("ar"));}
 void shellMetacharactersAreLiteral(){Command c; QSignalSpy done(&c,&Command::finished); c.run("/usr/bin/printf",{"%s","$(touch /tmp/harbor-should-not-exist)"}); QVERIFY(done.wait()); QCOMPARE(done[0][0].toBool(),true); QCOMPARE(done[0][1].toString(),QString("$(touch /tmp/harbor-should-not-exist)"));}
 void failedProgramEndsBusy(){Command c; QSignalSpy done(&c,&Command::finished); c.run("/missing/harbor",{}); QVERIFY(done.wait()); QVERIFY(!c.busy()); QVERIFY(!done[0][0].toBool());}
 void timeoutEndsProcess(){Command c; QSignalSpy done(&c,&Command::finished); c.run("/usr/bin/sleep",{"10"},40); QVERIFY(done.wait(2000)); QVERIFY(!done[0][0].toBool()); QVERIFY(!c.busy());}
};
QTEST_GUILESS_MAIN(CoreTest)
#include "core_test.moc"
