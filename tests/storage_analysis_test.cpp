#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <unistd.h>
#include <sys/stat.h>
#include "core/StorageSettings.h"
class AnalysisTest:public QObject{Q_OBJECT
private slots:void scan(){QTemporaryDir dir;QFile file(dir.filePath("file.txt"));QVERIFY(file.open(QIODevice::WriteOnly));file.write("1234567890");file.close();QVERIFY(QFile::link(dir.filePath("file.txt"),dir.filePath("link.txt")));QVERIFY(::link(QFile::encodeName(dir.filePath("file.txt")).constData(),QFile::encodeName(dir.filePath("hard.txt")).constData())==0);StorageSettings s;s.analyze(dir.path());QTRY_VERIFY_WITH_TIMEOUT(!s.analyzing(),3000);QCOMPARE(s.categories().size(),1);struct stat st{};QVERIFY(::stat(QFile::encodeName(dir.filePath("file.txt")).constData(),&st)==0);QCOMPARE(s.categories()[0].toMap()["bytes"].toLongLong(),qint64(st.st_blocks)*512);QCOMPARE(s.largestFiles().size(),1);s.analyze("/proc");QTRY_VERIFY(!s.analyzing());QVERIFY(!s.analysisMessage().isEmpty());}
};QTEST_GUILESS_MAIN(AnalysisTest)
#include "storage_analysis_test.moc"
