#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include "core/Applications.h"
class AppsTest: public QObject {
 Q_OBJECT
private slots:
 void hiddenEntriesStayHidden(){QTemporaryDir dir;qputenv("XDG_DATA_HOME",dir.path().toUtf8());QDir().mkpath(dir.path()+"/applications");QFile f(dir.path()+"/applications/harbor-hidden-test.desktop");QVERIFY(f.open(QIODevice::WriteOnly));f.write("[Desktop Entry]\nType=Application\nName=Harbor Hidden Test\nExec=/bin/true\nHidden=true\n");f.close();Applications a;for(auto v:a.entries())QVERIFY(v.toMap()["id"]!="harbor-hidden-test.desktop");}
 void pathsCannotBeLaunchedAsIds(){Applications a;QSignalSpy errors(&a,&Applications::error);QVERIFY(!a.launch("../../untrusted.desktop"));QCOMPARE(errors.size(),1);}
};
QTEST_GUILESS_MAIN(AppsTest)
#include "apps_test.moc"
