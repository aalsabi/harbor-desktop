#include <QtTest>
#include <QDBusVirtualObject>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include "core/Bluetooth.h"
class FakePairing : public QDBusVirtualObject {
    Q_OBJECT
public:
    QString agentService, agentPath;
    QString request = "RequestConfirmation";
    QDBusMessage pairing;
    bool accepted = false;
    int pairs = 0, removed = 0;
    const QString device = "/org/bluez/hci0/dev_00_11_22_33_44_55";
    QString introspect(const QString&) const override { return {}; }
    bool handleMessage(const QDBusMessage& message, const QDBusConnection& bus) override {
        const auto method = message.member();
        if (method == "GetManagedObjects") {
            BluezObjects objects{
                {QDBusObjectPath("/org/bluez/hci0"), {{"org.bluez.Adapter1", {{"Powered", true}}}}},
                {QDBusObjectPath(device),
                 {{"org.bluez.Device1",
                   {{"Address", "00:11:22:33:44:55"},
                    {"Alias", "Test keyboard"},
                    {"Adapter", QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0"))},
                    {"Paired", false}}}}}};
            bus.send(message.createReply({QVariant::fromValue(objects)}));
            return true;
        }
        if (method == "RegisterAgent") {
            agentService = message.service();
            agentPath = qdbus_cast<QDBusObjectPath>(message.arguments().first()).path();
        }
        if (method == "Pair") {
            ++pairs;
            pairing = message;
            auto call = QDBusMessage::createMethodCall(agentService, agentPath, "org.bluez.Agent1", request);
            QVariantList args{QVariant::fromValue(QDBusObjectPath(device))};
            if (request == "RequestConfirmation")
                args << uint(123456);
            call.setArguments(args);
            auto* watcher = new QDBusPendingCallWatcher(bus.asyncCall(call, 5000), this);
            connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, bus] {
                const auto reply = watcher->reply();
                accepted = reply.type() != QDBusMessage::ErrorMessage;
                if (request == "RequestPinCode" && accepted)
                    accepted = reply.arguments().first().toString() == "1234";
                bus.send(accepted ? pairing.createReply()
                                  : pairing.createErrorReply("org.bluez.Error.Rejected", "Rejected"));
                watcher->deleteLater();
            });
            return true;
        }
        if (method == "RemoveDevice")
            ++removed;
        bus.send(message.createReply());
        return true;
    }
};
class PairingTest : public QObject {
    Q_OBJECT
private slots:
    void nativeConfirmationAndPin() {
        auto bus = QDBusConnection::sessionBus();
        QVERIFY(bus.isConnected());
        FakePairing fake;
        auto fakeBus = QDBusConnection::connectToBus(QDBusConnection::SessionBus, "fake-bluez-pairing");
        QVERIFY(fakeBus.registerService("org.bluez"));
        QVERIFY(fakeBus.registerVirtualObject("/", &fake, QDBusConnection::SubPath));
        {
            Bluetooth bluetooth(nullptr, bus);
            QTRY_VERIFY(bluetooth.state().value("bluetoothAvailable").toBool());
            bluetooth.action("bluetooth-pair", "/not/a/device");
            QCOMPARE(fake.pairs, 0);
            bluetooth.action("bluetooth-pair", fake.device);
            QTRY_COMPARE(bluetooth.state().value("bluetoothPrompt").toMap().value("kind").toString(),
                         QString("confirm"));
            QVERIFY(!fake.accepted);
            auto intruder =
                QDBusConnection::connectToBus(QDBusConnection::SessionBus, "bluez-agent-intruder");
            auto request = QDBusMessage::createMethodCall(bus.baseService(), "/org/harbor/BluetoothAgent",
                                                          "org.bluez.Agent1", "DisplayPinCode");
            request.setArguments({QVariant::fromValue(QDBusObjectPath(fake.device)), QString("9999")});
            QDBusPendingCallWatcher denied(intruder.asyncCall(request, 1000));
            QTRY_VERIFY(denied.isFinished());
            QCOMPARE(denied.reply().type(), QDBusMessage::ErrorMessage);
            bluetooth.action("bluetooth-respond", QVariantMap{{"accept", false}});
            QTRY_VERIFY(!bluetooth.state().value("bluetoothBusy").toBool());
            QVERIFY(!fake.accepted);
            bluetooth.action("bluetooth-pair", fake.device);
            QTRY_COMPARE(bluetooth.state().value("bluetoothPrompt").toMap().value("kind").toString(),
                         QString("confirm"));
            bluetooth.action("bluetooth-respond", QVariantMap{{"accept", true}});
            QTRY_VERIFY(!bluetooth.state().value("bluetoothBusy").toBool());
            QVERIFY(fake.accepted);
            fake.request = "RequestPinCode";
            fake.accepted = false;
            bluetooth.action("bluetooth-pair", fake.device);
            QTRY_COMPARE(bluetooth.state().value("bluetoothPrompt").toMap().value("kind").toString(),
                         QString("pin"));
            bluetooth.action("bluetooth-respond", QVariantMap{{"accept", true}, {"value", "1234"}});
            QTRY_VERIFY(!bluetooth.state().value("bluetoothBusy").toBool());
            QVERIFY(fake.accepted);
            bluetooth.action("bluetooth-unpair", fake.device);
            QTRY_COMPARE(fake.removed, 1);
            QTRY_VERIFY(!bluetooth.state().value("bluetoothBusy").toBool());
        }
        fakeBus.unregisterObject("/", QDBusConnection::UnregisterTree);
        fakeBus.unregisterService("org.bluez");
    }
};
QTEST_GUILESS_MAIN(PairingTest)
#include "bluetooth_pairing_test.moc"
