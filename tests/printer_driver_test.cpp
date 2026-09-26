#include <QtTest>
#include <QDBusVirtualObject>
#include <QDBusObjectPath>
#include <QDBusMessage>
#include <QTimer>
#include "core/PrinterDrivers.h"
class PrinterPackageKitFake : public QDBusVirtualObject {
public:
    int counter = 0;
    QStringList methods;
    QList<QVariantList> updates;
    bool rejectPreview = false, requireTrust = false, previewRemoval = false;
    QString introspect(const QString&) const override { return {}; }
    void signal(const QDBusConnection& bus, const QString& path, const QString& name,
                const QVariantList& args) {
        auto m = QDBusMessage::createSignal(path, "org.freedesktop.PackageKit.Transaction", name);
        m.setArguments(args);
        bus.send(m);
    }
    bool handleMessage(const QDBusMessage& m, const QDBusConnection& bus) override {
        if (m.member() == "CreateTransaction") {
            QString path = "/org/freedesktop/PackageKit/transactions/t" + QString::number(++counter);
            bus.send(m.createReply(QVariantList{QVariant::fromValue(QDBusObjectPath(path))}));
            return true;
        }
        methods.append(m.member());
        if (m.member() == "SetHints") {
            bus.send(m.createReply());
            return true;
        }
        if (m.member() != "Resolve" && m.member() != "RefreshCache" && m.member() != "InstallPackages") {
            bus.send(
                m.createErrorReply("org.freedesktop.DBus.Error.UnknownMethod", "Unsupported fixture method"));
            return true;
        }
        if (m.member() == "InstallPackages")
            updates.append(m.arguments());
        bus.send(m.createReply());
        QTimer::singleShot(0, this, [this, m, bus] {
            auto path = m.path();
            bool isPreview = m.member() == "InstallPackages" &&
                             (m.arguments()[0].toULongLong() & PrinterDrivers::Simulate);
            if (isPreview && rejectPreview) {
                signal(bus, path, "ErrorCode", {uint(1), QString("Fixture backend cannot simulate")});
                signal(bus, path, "Finished", {uint(2), uint(1)});
                return;
            }
            if (m.member() == "Resolve") {
                signal(bus, path, "Package",
                       {uint(2), QString("printer-driver-hpcups;2.0;x86_64;repo"), QString("Security fix")});
                signal(bus, path, "Package",
                       {uint(9), QString("blocked;2.0;x86_64;updates"), QString("Blocked dependency")});
            }
            if (m.member() == "InstallPackages") {
                signal(bus, path, "Package",
                       {uint(11), QString("printer-driver-hpcups;2.0;x86_64;repo"), QString("Security fix")});
                if (isPreview) {
                    signal(bus, path, "Package",
                           {uint(12), QString("library;1.0;x86_64;updates"), QString("Dependency")});
                    if (previewRemoval)
                        signal(bus, path, "Package",
                               {uint(13), QString("old-library;0.9;x86_64;installed"),
                                QString("Replaced dependency")});
                } else
                    signal(bus, path, "RequireRestart",
                           {uint(4), QString("printer-driver-hpcups;2.0;x86_64;repo")});
                if (requireTrust) {
                    signal(bus, path, "EulaRequired",
                           {QString("license"), QString("printer-driver-hpcups;2.0;x86_64;repo"),
                            QString("Vendor"), QString("License terms")});
                    signal(bus, path, "Finished", {uint(6), uint(1)});
                    return;
                }
            }
            auto progress =
                QDBusMessage::createSignal(path, "org.freedesktop.DBus.Properties", "PropertiesChanged");
            progress.setArguments({QString("org.freedesktop.PackageKit.Transaction"),
                                   QVariantMap{{"Percentage", uint(75)}}, QStringList{}});
            bus.send(progress);
            signal(bus, path, "Finished", {uint(1), uint(1)});
        });
        return true;
    }
};
class PrinterDriverTest : public QObject {
    Q_OBJECT
    PrinterPackageKitFake fake;
    QDBusConnection bus = QDBusConnection::sessionBus();
    QString id = "printer-driver-hpcups;2.0;x86_64;repo";
private slots:
    void initTestCase() {
        QVERIFY(bus.isConnected());
        QVERIFY(bus.registerService("org.freedesktop.PackageKit"));
        QVERIFY(bus.registerVirtualObject("/", &fake, QDBusConnection::SubPath));
    }
    void init() {
        fake.methods.clear();
        fake.updates.clear();
        fake.rejectPreview = false;
        fake.requireTrust = false;
        fake.previewRemoval = false;
    }
    void cleanupTestCase() {
        bus.unregisterObject("/");
        bus.unregisterService("org.freedesktop.PackageKit");
    }
    void passiveConstruction() {
        PrinterDrivers updates(bus);
        QVERIFY(!updates.busy());
        QVERIFY(fake.methods.isEmpty());
    }
    void resolvesOnlyAllowlistedDrivers() {
        PrinterDrivers updates(bus);
        updates.search("HP");
        QTRY_VERIFY(!updates.busy());
        QCOMPARE(updates.packages().size(), 1);
        QVERIFY(updates.checked());
        QVERIFY(fake.methods.contains("Resolve"));
        QCOMPARE(updates.percentage(), 100);
        QVERIFY(fake.updates.isEmpty());
    }
    void previewThenConfirm() {
        PrinterDrivers updates(bus);
        updates.search("HP");
        QTRY_VERIFY(!updates.busy());
        updates.install(true);
        QVERIFY(fake.updates.isEmpty());
        updates.prepare({id});
        QTRY_VERIFY(!updates.busy());
        QVERIFY(updates.readyToInstall());
        QCOMPARE(updates.preview().size(), 2);
        QCOMPARE(fake.updates.size(), 1);
        QCOMPARE(fake.updates[0][0].toULongLong(), qulonglong(6));
        updates.install(false);
        QCOMPARE(fake.updates.size(), 1);
        updates.install(true);
        QTRY_VERIFY(!updates.busy());
        QCOMPARE(fake.updates.size(), 2);
        QCOMPARE(fake.updates[1][0].toULongLong(), qulonglong(2));
        QVERIFY(!updates.readyToInstall());
        QVERIFY(updates.restart().contains("computer"));
    }
    void rejectsUnknownAndBlocked() {
        PrinterDrivers updates(bus);
        updates.search("HP");
        QTRY_VERIFY(!updates.busy());
        updates.prepare({"blocked;2.0;x86_64;updates"});
        QVERIFY(fake.updates.isEmpty());
        updates.prepare({"unknown;1;x86_64;repo"});
        QVERIFY(fake.updates.isEmpty());
        QVERIFY(!updates.error().isEmpty());
    }
    void unsupportedSimulationNeverInstalls() {
        fake.rejectPreview = true;
        PrinterDrivers updates(bus);
        updates.search("HP");
        QTRY_VERIFY(!updates.busy());
        updates.prepare({id});
        QTRY_VERIFY(!updates.busy());
        QVERIFY(!updates.readyToInstall());
        QVERIFY(updates.error().contains("cannot simulate"));
        updates.install(true);
        QCOMPARE(fake.updates.size(), 1);
        QCOMPARE(fake.updates[0][0].toULongLong(), qulonglong(6));
    }
    void licenseNeverAccepted() {
        fake.requireTrust = true;
        PrinterDrivers updates(bus);
        updates.search("HP");
        QTRY_VERIFY(!updates.busy());
        updates.prepare({id});
        QTRY_VERIFY(!updates.busy());
        QVERIFY(!updates.readyToInstall());
        QVERIFY(updates.error().contains("has not accepted"));
        QVERIFY(!fake.methods.contains("AcceptEula"));
        QVERIFY(!fake.methods.contains("InstallSignature"));
    }
    void removalVisibleAndDiscardInvalidates() {
        fake.previewRemoval = true;
        PrinterDrivers updates(bus);
        updates.search("HP");
        QTRY_VERIFY(!updates.busy());
        updates.prepare({id});
        QTRY_VERIFY(!updates.busy());
        QCOMPARE(updates.preview().size(), 3);
        QCOMPARE(updates.preview()[2].toMap().value("action").toString(), QString("Remove"));
        updates.discardPreview();
        QVERIFY(!updates.readyToInstall());
        updates.install(true);
        QCOMPARE(fake.updates.size(), 1);
    }
};
QTEST_GUILESS_MAIN(PrinterDriverTest)
#include "printer_driver_test.moc"
