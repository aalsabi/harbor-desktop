#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QDBusVirtualObject>
#include <QDBusMessage>
#include <unistd.h>
#include "core/Printers.h"
#include "core/PrinterDrivers.h"
class PrinterAdminFake : public QDBusVirtualObject {
public:
    QStringList methods;
    QList<QVariantList> arguments;
    bool reject = false;
    QString introspect(const QString&) const override { return {}; }
    bool handleMessage(const QDBusMessage& m, const QDBusConnection& bus) override {
        methods.append(m.member());
        arguments.append(m.arguments());
        bus.send(reject ? m.createErrorReply("org.opensuse.CupsPkHelper.Mechanism.NotPrivileged",
                                             "Fixture authorization denied")
                        : m.createReply(QVariantList{QString()}));
        return true;
    }
};
class PrintersTest : public QObject {
    Q_OBJECT
    QTemporaryDir fixture;
    QByteArray oldPath;
    bool hadPath = false;
    void write(const QString& name, const QByteArray& body) {
        QFile f(fixture.filePath(name));
        QVERIFY(f.open(QIODevice::WriteOnly));
        QCOMPARE(f.write(body), body.size());
        f.close();
        QVERIFY(f.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
    }
    QByteArray read(const QString& name) {
        QFile f(fixture.filePath(name));
        if (!f.open(QIODevice::ReadOnly))
            return {};
        return f.readAll();
    }
private slots:
    void initTestCase() {
        QVERIFY(fixture.isValid());
        hadPath = qEnvironmentVariableIsSet("PATH");
        oldPath = qgetenv("PATH");
        qputenv("PATH", fixture.path().toUtf8());
        qputenv("HARBOR_PRINTER_TEST_DIR", fixture.path().toUtf8());

#ifndef Q_MOC_RUN
        write("lpstat", R"SCRIPT(#!/bin/sh
[ "$1" = '-h' ] && [ "$2" = 'localhost' ] || exit 8
shift 2
printf 'lpstat %s %s\n' "$1" "$2" >> "$HARBOR_PRINTER_TEST_DIR/log"
if [ "$HARBOR_PRINTER_TEST_FAIL" = "$1" ]; then printf 'fixture command failure\n' >&2; exit 1; fi
if [ "$1" = '-p' ]; then
 printf 'printer Office is idle. enabled since Wed\nprinter طابعة;$(id) is idle. enabled since Wed\n'
 default=Office
 if [ -f "$HARBOR_PRINTER_TEST_DIR/default" ]; then IFS= read -r default < "$HARBOR_PRINTER_TEST_DIR/default"; fi
 printf 'system default destination: %s\n' "$default"
elif [ "$1" = '-o' ]; then
 printf 'Office-42 user 2048 Wed 23 Sep\n'
else exit 9; fi
)SCRIPT");
        write("lpinfo", R"SCRIPT(#!/bin/sh
if [ "$4" = '--timeout' ] && [ "$5" = '8' ] && [ "$6" = '-v' ]; then
 printf 'Device: ipp://office.local/ipp/print\n    class = network\n    info = Office Laser\n    make-and-model = HP LaserJet\n    device-id = MFG:HP;MDL:LaserJet;\nDevice: usb://HP/LaserJet?serial=123\n    info = USB Laser\n    make-and-model = HP LaserJet\nDevice: file:///etc/passwd\n    info = Unsafe\n'
else printf 'drv:///hp.drv/laser.ppd HP LaserJet\n'; fi
)SCRIPT");
        write("lpoptions", R"SCRIPT(#!/bin/sh
[ "$1" = '-h' ] && [ "$2" = 'localhost' ] || exit 8
shift 2
printf 'lpoptions %s %s\n' "$1" "$2" >> "$HARBOR_PRINTER_TEST_DIR/log"
if [ "$HARBOR_PRINTER_TEST_FAIL" = 'set' ]; then printf 'fixture save failure\n' >&2; exit 1; fi
if [ "$1" = '-p' ] && [ "$3" = '-l' ]; then printf 'Duplex/Two-sided: *None DuplexNoTumble DuplexTumble\nPageSize/Paper: A4 *Letter\n'; exit 0; fi
if [ "$1" = '-p' ] && [ "$3" = '-o' ]; then printf '%s\n' "$4" > "$HARBOR_PRINTER_TEST_DIR/option"; exit 0; fi
[ "$1" = '-d' ] && [ "$#" = 2 ] || exit 9
printf '%s\n' "$2" > "$HARBOR_PRINTER_TEST_DIR/default"
)SCRIPT");
#endif
    }
    void cleanup() {
        QFile::remove(fixture.filePath("option"));
        qunsetenv("HARBOR_PRINTER_TEST_FAIL");
        QFile::remove(fixture.filePath("log"));
        QFile::remove(fixture.filePath("default"));
    }
    void cleanupTestCase() {
        if (hadPath)
            qputenv("PATH", oldPath);
        else
            qunsetenv("PATH");
        qunsetenv("HARBOR_PRINTER_TEST_DIR");
    }

    void driverPackageValidation() {
        PrinterDrivers drivers;
        QVERIFY(QMetaObject::invokeMethod(&drivers, "search", Q_ARG(QString, QString("arbitrary-package"))));
        QVERIFY(!drivers.property("error").toString().isEmpty());
        QVERIFY(!drivers.property("busy").toBool());
    }
    void discoversDevices() {
        Printers printers;
        QVERIFY(QMetaObject::invokeMethod(&printers, "discover"));
        QTRY_VERIFY(!printers.busy());
        auto rows = printers.property("discovered").toList();
        QCOMPARE(rows.size(), 2);
        QCOMPARE(rows[0].toMap().value("name").toString(), QString("Office Laser"));
        QCOMPARE(rows[0].toMap().value("deviceId").toString(), QString("MFG:HP;MDL:LaserJet;"));
        QCOMPARE(rows[1].toMap().value("uri").toString(), QString("usb://HP/LaserJet?serial=123"));
        printers.selectDevice(rows[1].toMap().value("uri").toString());
        QTRY_VERIFY(!printers.busy());
        QCOMPARE(printers.drivers().size(), 1);
        QCOMPARE(printers.selectedDevice().value("name").toString(), QString("USB Laser"));
    }
    void printerParsing() {
        auto rows = Printers::parsePrinters(
            "printer Office-2 is idle. enabled since Tue\n\tReady\nprinter Color disabled since Tue -\n");
        QCOMPARE(rows.size(), 2);
        QCOMPARE(rows[0].toMap().value("name").toString(), QString("Office-2"));
        QVERIFY(rows[1].toMap().value("status").toString().startsWith("disabled"));
    }
    void defaults() {
        QCOMPARE(Printers::parseDefault("printer Office is idle\nsystem default destination: Office\n"),
                 QString("Office"));
        QVERIFY(Printers::parseDefault("no system default destination").isEmpty());
    }
    void jobs() {
        auto rows = Printers::parseJobs("Office-West-123 user 1024 Wed 23 Sep\nmalformed\n");
        QCOMPARE(rows.size(), 1);
        QCOMPARE(rows[0].toMap().value("printer").toString(), QString("Office-West"));
        QCOMPARE(rows[0].toMap().value("id").toString(), QString("Office-West-123"));
    }
    void names() {
        for (const auto& name : QStringList{"Office-2.local", "طابعة", "a;b", "$(id)"})
            QVERIFY(Printers::validName(name));
        for (const auto& name : QStringList{"-h", "a/b", "a b", "a#b", "a\tb", "a\nb", ""})
            QVERIFY(!Printers::validName(name));
    }
    void refreshWorkflow() {
        Printers printers;
        printers.refresh();
        QVERIFY(printers.busy());
        QTRY_VERIFY(!printers.busy());
        QVERIFY(printers.available());
        QCOMPARE(printers.devices().size(), 2);
        QCOMPARE(printers.defaultPrinter(), QString("Office"));
        QCOMPARE(printers.devices()[0].toMap().value("jobs").toList().size(), 1);
        QVERIFY(printers.message().isEmpty());
        QCOMPARE(read("log"), QByteArray("lpstat -p -d\nlpstat -o \n"));
    }
    void defaultWorkflow() {
        Printers printers;
        printers.refresh();
        QTRY_VERIFY(!printers.busy());
        QFile::remove(fixture.filePath("log"));
        printers.setDefault(QString::fromUtf8("طابعة;$(id)"));
        if (geteuid() == 0) {
            QVERIFY(!printers.busy());
            QVERIFY(printers.message().contains("root"));
            QVERIFY(read("log").isEmpty());
            return;
        }
        QTRY_VERIFY(!printers.busy());
        QCOMPARE(printers.defaultPrinter(), QString::fromUtf8("طابعة;$(id)"));
        QCOMPARE(read("default"), QString::fromUtf8("طابعة;$(id)\n").toUtf8());
        QCOMPARE(read("log"),
                 QString::fromUtf8("lpoptions -d طابعة;$(id)\nlpstat -p -d\nlpstat -o \n").toUtf8());
    }
    void listingFailure() {
        qputenv("HARBOR_PRINTER_TEST_FAIL", "-p");
        Printers printers;
        printers.refresh();
        QTRY_VERIFY(!printers.busy());
        QVERIFY(printers.devices().isEmpty());
        QVERIFY(printers.message().contains("fixture command failure"));
        QCOMPARE(read("log"), QByteArray("lpstat -p -d\n"));
    }
    void queueFailure() {
        qputenv("HARBOR_PRINTER_TEST_FAIL", "-o");
        Printers printers;
        printers.refresh();
        QTRY_VERIFY(!printers.busy());
        QCOMPARE(printers.devices().size(), 2);
        QVERIFY(printers.message().contains("queue could not be read"));
    }
    void saveFailure() {
        if (geteuid() == 0)
            QSKIP("Root cannot set personal printer defaults");
        Printers printers;
        printers.refresh();
        QTRY_VERIFY(!printers.busy());
        qputenv("HARBOR_PRINTER_TEST_FAIL", "set");
        printers.setDefault("Office");
        QTRY_VERIFY(!printers.busy());
        QCOMPARE(printers.defaultPrinter(), QString("Office"));
        QVERIFY(printers.message().contains("fixture save failure"));
        QVERIFY(read("default").isEmpty());
    }

    void optionParsingAndValidation() {
        auto options = Printers::parseOptions(
            "Duplex/Two-sided: *None DuplexNoTumble DuplexTumble\nPageSize/Paper: A4 *Letter\n");
        QCOMPARE(options.size(), 2);
        QCOMPARE(options[1].toMap().value("currentIndex").toInt(), 1);
        QVERIFY(Printers::validUri("ipps://printer.local/ipp/print"));
        for (const auto& uri : QStringList{"file:///etc/passwd", "ipp://user:password@host/ipp",
                                           "ipp://host/ipp?a=b", "ipp://host/a b", "ipp://"})
            QVERIFY(!Printers::validUri(uri));
        auto drivers = Printers::parseDrivers("drv:///sample.drv/model.ppd Example Model\n");
        QCOMPARE(drivers.size(), 1);
    }
    void optionsWorkflow() {
        Printers printers;
        printers.refresh();
        QTRY_VERIFY(!printers.busy());
        printers.loadOptions("Office");
        QTRY_VERIFY(!printers.busy());
        QCOMPARE(printers.options().size(), 2);
        printers.setOption("Office", "Duplex", "invalid;option");
        QVERIFY(!printers.busy());
        QVERIFY(!printers.message().isEmpty());
        if (geteuid() == 0)
            return;
        printers.setOption("Office", "Duplex", "DuplexNoTumble");
        QTRY_VERIFY(!printers.busy());
        QCOMPARE(read("option"), QByteArray("Duplex=DuplexNoTumble\n"));
    }
    void authorizedAdminWorkflow() {
        auto bus = QDBusConnection::sessionBus();
        QVERIFY(bus.isConnected());
        PrinterAdminFake fake;
        QVERIFY(bus.registerService("org.opensuse.CupsPkHelper.Mechanism"));
        QVERIFY(bus.registerVirtualObject("/", &fake, QDBusConnection::SubPath));
        {
            Printers printers(bus);
            printers.refresh();
            QTRY_VERIFY(!printers.busy());
            printers.addPrinter("New_Printer", "ipps://printer.local/ipp/print", "everywhere");
            QTRY_VERIFY(!printers.busy());
            QCOMPARE(fake.methods, QStringList({"PrinterAdd", "PrinterSetEnabled", "PrinterSetAcceptJobs"}));
            QCOMPARE(fake.arguments[0],
                     QVariantList({QString("New_Printer"), QString("ipps://printer.local/ipp/print"),
                                   QString("everywhere"), QString("New_Printer"), QString()}));
            fake.methods.clear();
            printers.removePrinter("Office", false);
            QVERIFY(fake.methods.isEmpty());
            printers.cancelJob("Office-42", false);
            QVERIFY(fake.methods.isEmpty());
            printers.cancelJob("Office-999", true);
            QVERIFY(fake.methods.isEmpty());
            printers.cancelJob("Office-42", true);
            QTRY_VERIFY(!printers.busy());
            QCOMPARE(fake.methods, QStringList{"JobCancelPurge"});
            QCOMPARE(fake.arguments.last(), QVariantList({42, false}));
            printers.removePrinter("Office", true);
            QTRY_VERIFY(!printers.busy());
            QCOMPARE(fake.methods.last(), QString("PrinterDelete"));
            fake.reject = true;
            printers.removePrinter("Office", true);
            QTRY_VERIFY(!printers.busy());
            QVERIFY(printers.message().contains("authorization denied"));
        }
        bus.unregisterObject("/");
        bus.unregisterService("org.opensuse.CupsPkHelper.Mechanism");
    }
    void rejectsUnknownPrinter() {
        Printers printers;
        printers.setDefault("Unknown");
        QVERIFY(!printers.message().isEmpty());
        QVERIFY(!printers.busy());
    }
};
QTEST_GUILESS_MAIN(PrintersTest)
#include "printers_test.moc"
