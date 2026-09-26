#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <sys/stat.h>
#include <unistd.h>
#include "core/ApplicationStorage.h"
class ApplicationStorageTest : public QObject {
    Q_OBJECT
    static void put(const QString& path, int size = 8192) {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        QCOMPARE(f.write(QByteArray(size, 'x')), qint64(size));
    }
    static qint64 allocated(const QString& path) {
        struct stat s{};
        if (::lstat(QFile::encodeName(path).constData(), &s))
            return -1;
        return s.st_blocks * 512;
    }
    static QVariantMap app(const QVariantMap& result, const QString& id) {
        for (const auto& v : result.value("apps").toList())
            if (v.toMap().value("id") == id)
                return v.toMap();
        return {};
    }
private slots:
    void inventoriesExactPackageFilesAndConfirmedFolders() {
        QTemporaryDir dir;
        const auto base = dir.path();
        for (const auto& name : {"apps", "data", "cache", "config", "native-data", "package"})
            QDir().mkpath(base + "/" + name);
        QFile desktop(base + "/apps/org.example.Demo.desktop");
        QVERIFY(desktop.open(QIODevice::WriteOnly));
        desktop.write("[Desktop Entry]\nType=Application\nName=Demo\nExec=demo\n");
        desktop.close();
        put(base + "/package/binary");
        put(base + "/native-data/settings");
        put(base + "/data/unassigned");
        QFile fake(base + "/fake-dpkg");
        QVERIFY(fake.open(QIODevice::WriteOnly));
        fake.write(("#!/bin/sh\ncase \"$1\" in\n-S) printf '%s\\n' 'demo: " + base +
                    "/apps/org.example.Demo.desktop';;\n-L) printf '%s\\n' '" + base +
                    "/package/binary';;\nesac\n")
                       .toUtf8());
        fake.close();
        QVERIFY(fake.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        ApplicationStorage::Options options;
        options.home = base;
        options.dataHome = base + "/data";
        options.cacheHome = base + "/cache";
        options.configHome = base + "/config";
        options.associationsFile = base + "/config/associations.json";
        options.applicationDirs = {base + "/apps"};
        options.dpkgProgram = fake.fileName();
        options.flatpakProgram = "/nonexistent-fixture-flatpak";
        ApplicationStorage storage(options);
        storage.measure();
        QTRY_VERIFY_WITH_TIMEOUT(!storage.busy(), 5000);
        QCOMPARE(storage.applications().size(), 1);
        auto row = storage.applications().first().toMap();
        QCOMPARE(row.value("name").toString(), QString("Demo"));
        QCOMPARE(row.value("installedBytes").toLongLong(), allocated(base + "/package/binary"));
        QVERIFY(!row.value("dataKnown").toBool());
        storage.associateFolder(row.value("id").toString(), base + "/native-data", "data");
        QTRY_VERIFY_WITH_TIMEOUT(!storage.busy(), 5000);
        QCOMPARE(storage.associations().size(), 1);
        QCOMPARE(storage.applications().first().toMap().value("dataBytes").toLongLong(),
                 allocated(base + "/native-data/settings"));
        QVERIFY(QFileInfo::exists(options.associationsFile));
        storage.associateFolder("not-a-listed-app", base + "/data", "data");
        QVERIFY(!storage.error().isEmpty());
        QCOMPARE(storage.associations().size(), 1);
        QVERIFY(QFile::link(base + "/native-data", base + "/linked-data"));
        storage.associateFolder(row.value("id").toString(), base + "/linked-data", "data");
        QTRY_VERIFY_WITH_TIMEOUT(!storage.busy(), 5000);
        QVERIFY(!storage.error().isEmpty());
        QCOMPARE(storage.associations().size(), 1);
        storage.associateFolder(row.value("id").toString(), base + "/native-data/nested", "cache");
        QVERIFY(!storage.error().isEmpty());
        QVERIFY(!storage.busy());
        storage.removeAssociation(row.value("id").toString(), base + "/native-data");
        QTRY_VERIFY_WITH_TIMEOUT(!storage.busy(), 5000);
        QVERIFY(storage.associations().isEmpty());
        QVERIFY(QFileInfo::exists(base + "/native-data/settings"));
    }
    void flatpakIncludesDeploymentDataAndCache() {
        QTemporaryDir dir;
        const auto base = dir.path();
        for (const auto& name : {"apps", "deployment/files", ".var/app/org.example.Demo/data",
                                 ".var/app/org.example.Demo/config", ".var/app/org.example.Demo/cache",
                                 "data", "config", "cache"})
            QDir().mkpath(base + "/" + name);
        put(base + "/deployment/files/binary");
        put(base + "/.var/app/org.example.Demo/data/document");
        put(base + "/.var/app/org.example.Demo/cache/cached");
        QFile fake(base + "/fake-flatpak");
        QVERIFY(fake.open(QIODevice::WriteOnly));
        fake.write(
            ("#!/bin/sh\ncase \"$1\" in\nlist) printf 'org.example.Demo\\tDemo\\tuser\\n';;\ninfo) printf '%s\\n' '" +
             base + "/deployment';;\nesac\n")
                .toUtf8());
        fake.close();
        QVERIFY(fake.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        ApplicationStorage::Options options;
        options.home = base;
        options.dataHome = base + "/data";
        options.cacheHome = base + "/cache";
        options.configHome = base + "/config";
        options.associationsFile = base + "/config/associations.json";
        options.applicationDirs = {base + "/apps"};
        options.dpkgProgram = "/nonexistent-dpkg";
        options.flatpakProgram = fake.fileName();
        ApplicationStorage storage(options);
        storage.measure();
        QTRY_VERIFY_WITH_TIMEOUT(!storage.busy(), 5000);
        QCOMPARE(storage.applications().size(), 1);
        auto row = storage.applications().first().toMap();
        QCOMPARE(row.value("installedBytes").toLongLong(), allocated(base + "/deployment/files/binary"));
        QCOMPARE(row.value("dataBytes").toLongLong(),
                 allocated(base + "/.var/app/org.example.Demo/data/document"));
        QCOMPARE(row.value("cacheBytes").toLongLong(),
                 allocated(base + "/.var/app/org.example.Demo/cache/cached"));
        QVERIFY(row.value("dataKnown").toBool());
    }
    void cancelStopsCommandWork() {
        QTemporaryDir dir;
        const auto base = dir.path();
        QDir().mkdir(base + "/apps");
        QFile desktop(base + "/apps/org.example.Slow.desktop");
        QVERIFY(desktop.open(QIODevice::WriteOnly));
        desktop.write("[Desktop Entry]\nType=Application\nName=Slow\n");
        desktop.close();
        QFile fake(base + "/slow-command");
        QVERIFY(fake.open(QIODevice::WriteOnly));
        fake.write("#!/usr/bin/python3\nimport time\ntime.sleep(10)\n");
        fake.close();
        QVERIFY(fake.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        ApplicationStorage::Options options;
        options.home = base;
        options.dataHome = base + "/data";
        options.cacheHome = base + "/cache";
        options.configHome = base + "/config";
        options.associationsFile = base + "/config/associations.json";
        options.applicationDirs = {base + "/apps"};
        options.dpkgProgram = fake.fileName();
        options.flatpakProgram = "/nonexistent-flatpak";
        ApplicationStorage storage(options);
        storage.measure();
        QVERIFY(storage.busy());
        QTest::qWait(100);
        storage.cancel();
        QTRY_VERIFY_WITH_TIMEOUT(!storage.busy(), 1500);
        QVERIFY(storage.summary().value("canceled").toBool());
    }
    void measuredBytesDeduplicateHardlinks() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        put(dir.path() + "/file");
        QVERIFY(::link(QFile::encodeName(dir.path() + "/file").constData(),
                       QFile::encodeName(dir.path() + "/alias").constData()) == 0);
        auto result = ApplicationStorage::measureRoots(
            {QVariantMap{{"path", dir.path()}, {"owners", QStringList{"one"}}, {"kind", "data"}}});
        QCOMPARE(app(result, "one").value("dataBytes").toLongLong(), allocated(dir.path() + "/file"));
        QCOMPARE(result.value("sharedBytes").toLongLong(), qint64(0));
    }
    void sameAppHardlinkAcrossDataAndCacheIsStillAttributed() {
        QTemporaryDir dir;
        put(dir.path() + "/data");
        QVERIFY(::link(QFile::encodeName(dir.path() + "/data").constData(),
                       QFile::encodeName(dir.path() + "/cache").constData()) == 0);
        const auto result = ApplicationStorage::measureRoots({QVariantMap{{"path", dir.path() + "/data"},
                                                                          {"owners", QStringList{"one"}},
                                                                          {"kind", "data"},
                                                                          {"fileOnly", true}},
                                                              QVariantMap{{"path", dir.path() + "/cache"},
                                                                          {"owners", QStringList{"one"}},
                                                                          {"kind", "cache"},
                                                                          {"fileOnly", true}}});
        QCOMPARE(app(result, "one").value("totalBytes").toLongLong(), allocated(dir.path() + "/data"));
        QCOMPARE(app(result, "one").value("dataBytes").toLongLong(), allocated(dir.path() + "/data"));
        QCOMPARE(app(result, "one").value("cacheBytes").toLongLong(), qint64(0));
        QCOMPARE(result.value("sharedBytes").toLongLong(), qint64(0));
    }
    void sparseFileUsesAllocatedSpace() {
        QTemporaryDir dir;
        QFile file(dir.path() + "/sparse");
        QVERIFY(file.open(QIODevice::WriteOnly));
        QVERIFY(file.resize(16 * 1024 * 1024));
        file.close();
        const auto result =
            ApplicationStorage::measureRoots({QVariantMap{{"path", file.fileName()},
                                                          {"owners", QStringList{"sparse-app"}},
                                                          {"kind", "data"},
                                                          {"fileOnly", true}}});
        QCOMPARE(app(result, "sparse-app").value("dataBytes").toLongLong(), allocated(file.fileName()));
        QVERIFY(allocated(file.fileName()) < QFileInfo(file).size());
    }
    void sharedInstalledFilesCountOnce() {
        QTemporaryDir dir;
        put(dir.path() + "/binary");
        const auto result =
            ApplicationStorage::measureRoots({QVariantMap{{"path", dir.path() + "/binary"},
                                                          {"fileOnly", true},
                                                          {"owners", QStringList{"one", "two"}},
                                                          {"kind", "installed"}}});
        QCOMPARE(result.value("sharedBytes").toLongLong(), allocated(dir.path() + "/binary"));
        QCOMPARE(app(result, "one").value("installedBytes").toLongLong(), qint64(0));
        QCOMPARE(app(result, "two").value("sharedReferencedBytes").toLongLong(),
                 allocated(dir.path() + "/binary"));
    }
    void excludesSymlinkParentsAndChildren() {
        QTemporaryDir dir;
        QDir().mkdir(dir.path() + "/inside");
        QDir().mkdir(dir.path() + "/outside");
        put(dir.path() + "/outside/private");
        QVERIFY(QFile::link(dir.path() + "/outside", dir.path() + "/inside/link"));
        QVERIFY(QFile::link(dir.path() + "/outside", dir.path() + "/parent-link"));
        const auto result = ApplicationStorage::measureRoots(
            {QVariantMap{{"path", dir.path() + "/inside"}, {"owners", QStringList{"one"}}, {"kind", "data"}},
             QVariantMap{{"path", dir.path() + "/parent-link/private"},
                         {"fileOnly", true},
                         {"owners", QStringList{"one"}},
                         {"kind", "data"}}});
        QCOMPARE(result.value("measuredBytes").toLongLong(), qint64(0));
        QVERIFY(result.value("skipped").toLongLong() > 0);
    }
    void unattributedAndBounded() {
        QTemporaryDir dir;
        put(dir.path() + "/a");
        put(dir.path() + "/b");
        auto result =
            ApplicationStorage::measureRoots({QVariantMap{{"path", dir.path()}, {"kind", "cache"}}});
        QCOMPARE(result.value("unattributedCacheBytes").toLongLong(),
                 allocated(dir.path() + "/a") + allocated(dir.path() + "/b"));
        result = ApplicationStorage::measureRoots({QVariantMap{{"path", dir.path()}, {"kind", "cache"}}}, 1);
        QVERIFY(result.value("capped").toBool());
        QVERIFY(result.value("scannedFiles").toULongLong() <= 1);
    }
};
QTEST_GUILESS_MAIN(ApplicationStorageTest)
#include "application_storage_test.moc"
