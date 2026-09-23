#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include "core/Files.h"
class FilesTest: public QObject {
 Q_OBJECT
private slots:
 void navigationAndTabs(){QTemporaryDir dir;QDir(dir.path()).mkdir("child");Files f;f.navigate(dir.path());f.navigate(dir.filePath("child"));f.back();QCOMPARE(f.path(),dir.path());f.forward();QCOMPARE(f.path(),dir.filePath("child"));f.addTab();f.up();f.selectTab(0);QCOMPARE(f.path(),dir.filePath("child"));f.navigate("/missing/harbor-test");QCOMPARE(f.path(),dir.filePath("child"));QVERIFY(!f.error().isEmpty());}
 void brokenSymlinkVisible(){QTemporaryDir d;QFile::link(d.filePath("missing"),d.filePath("link"));Files f;f.navigate(d.path());QCOMPARE(f.entries().size(),1);QVERIFY(f.entries()[0].toMap()["link"].toBool());}
 void filterHiddenAndPreview(){QTemporaryDir dir;for(auto name:{"visible.txt",".hidden"}){QFile file(dir.filePath(name));QVERIFY(file.open(QIODevice::WriteOnly));file.write("hello");}Files f;f.navigate(dir.path());QCOMPARE(f.entries().size(),1);f.setHidden(true);QCOMPARE(f.entries().size(),2);f.setSearch("visible");QCOMPARE(f.entries().size(),1);QCOMPARE(f.preview(dir.filePath("visible.txt"))["text"].toString(),QString("hello"));}
};
QTEST_GUILESS_MAIN(FilesTest)
#include "files_test.moc"
