#include <QtTest>
#include <QTemporaryFile>
#include <QDBusVirtualObject>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include "core/NetworkSettings.h"
class FakeNetwork : public QDBusVirtualObject {
    Q_OBJECT
public:
    const QString base = "/org/freedesktop/NetworkManager";
    NetworkProfile saved{{"connection",
                          {{"id", "Test wired"},
                           {"uuid", "11111111-1111-1111-1111-111111111111"},
                           {"type", "802-3-ethernet"}}},
                         {"ipv4", {{"method", "auto"}}},
                         {"ipv6", {{"method", "auto"}}}};
    NetworkProfile written;
    int writes = 0;
    bool denied = false;
    QString agentService, receivedSecret, requestedSetting = "vpn";
    QStringList requestedHints{"password"};
    QString introspect(const QString&) const override { return {}; }
    bool handleMessage(const QDBusMessage& message, const QDBusConnection& bus) override {
        QVariantList reply;
        const auto path = message.path(), method = message.member();
        if (method == "GetAll") {
            QVariantMap p;
            if (path == base)
                p = {{"WirelessEnabled", true},
                     {"Devices",
                      QVariant::fromValue(QList<QDBusObjectPath>{QDBusObjectPath(base + "/Devices/1")})}};
            else if (path == base + "/Devices/1" &&
                     message.arguments().first().toString().endsWith(".Wireless"))
                p = {{"AccessPoints",
                      QVariant::fromValue(QList<QDBusObjectPath>{QDBusObjectPath(base + "/AccessPoint/1")})}};
            else if (path == base + "/Devices/1")
                p = {
                    {"Interface", "wlan0"}, {"DeviceType", uint(2)}, {"State", uint(100)}, {"Managed", true}};
            else
                p = {{"Ssid", QByteArray("Fixture WiFi")},
                     {"Strength", uchar(80)},
                     {"RsnFlags", uint(0x100)},
                     {"Flags", uint(1)}};
            reply << p;
        } else if (method == "RegisterWithCapabilities")
            agentService = message.service();
        else if (method == "ActivateConnection") {
            auto request =
                QDBusMessage::createMethodCall(agentService, "/org/harbor/NetworkAgent",
                                               "org.freedesktop.NetworkManager.SecretAgent", "GetSecrets");
            request.setArguments({QVariant::fromValue(saved),
                                  QVariant::fromValue(QDBusObjectPath(base + "/Settings/1")),
                                  requestedSetting, requestedHints, uint(1)});
            auto* watcher = new QDBusPendingCallWatcher(bus.asyncCall(request, 5000), this);
            connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
                const auto r = watcher->reply();
                if (r.type() != QDBusMessage::ErrorMessage) {
                    const auto settings = qdbus_cast<NetworkProfile>(r.arguments().first());
                    receivedSecret =
                        qdbus_cast<QMap<QString, QString>>(settings.value("vpn").value("secrets"))
                            .value("password");
                }
                watcher->deleteLater();
            });
            reply << QVariant::fromValue(QDBusObjectPath(base + "/ActiveConnection/1"));
        } else if (method == "GetSecrets" && message.arguments().first().toString() == "802-1x")
            reply << QVariant::fromValue(NetworkProfile{
                {"802-1x",
                 {{"password", "existing-eap-secret"}, {"private-key-password", "existing-key-secret"}}}});
        else if (method == "GetSecrets")
            reply << QVariant::fromValue(NetworkProfile{
                {"vpn",
                 {{"secrets", QVariant::fromValue(QMap<QString, QString>{{"password", "existing-secret"},
                                                                         {"cert-pass", "keep-cert"}})}}}});
        else if (method == "ListConnections")
            reply << QVariant::fromValue(QList<QDBusObjectPath>{QDBusObjectPath(base + "/Settings/1")});
        else if (method == "GetSettings")
            reply << QVariant::fromValue(saved);
        else if (method == "Update" || method == "AddConnection" || method == "AddAndActivateConnection") {
            ++writes;
            if (denied) {
                bus.send(message.createErrorReply("org.freedesktop.NetworkManager.PermissionDenied",
                                                  "Fixture denied"));
                return true;
            }
            written = qdbus_cast<NetworkProfile>(message.arguments().first());
            if (method == "Update")
                saved = written;
            if (method != "Update")
                reply << QVariant::fromValue(QDBusObjectPath(base + "/Settings/2"));
            if (method == "AddAndActivateConnection")
                reply << QVariant::fromValue(QDBusObjectPath(base + "/ActiveConnection/2"));
        }
        bus.send(message.createReply(reply));
        return true;
    }
};
class NetworkTest : public QObject {
    Q_OBJECT
private slots:
    void validation() {
        QVariantMap draft{{"ipv4Method", "manual"},
                          {"ipv4Addresses", "192.168.1.5/24, 10.0.0.3/8"},
                          {"ipv4Gateway", "192.168.1.1"},
                          {"ipv4Dns", "1.1.1.1"}};
        QVERIFY(NetworkSettings::validateIp(draft, 4).isEmpty());
        draft["ipv4Addresses"] = "192.168.1.2/33";
        QVERIFY(!NetworkSettings::validateIp(draft, 4).isEmpty());
        draft["ipv4Addresses"] = "::1/24";
        QVERIFY(!NetworkSettings::validateIp(draft, 4).isEmpty());
        QVariantMap six{{"ipv6Method", "manual"},
                        {"ipv6Addresses", "2001:db8::2/64"},
                        {"ipv6Gateway", "2001:db8::1"},
                        {"ipv6Dns", "2606:4700:4700::1111"}};
        QVERIFY(NetworkSettings::validateIp(six, 6).isEmpty());
        six["ipv6Addresses"] = "2001:db8::1/129";
        QVERIFY(!NetworkSettings::validateIp(six, 6).isEmpty());
    }
    void enterpriseProfiles() {
        auto bus = QDBusConnection::sessionBus();
        FakeNetwork fake;
        auto service = QDBusConnection::connectToBus(QDBusConnection::SessionBus, "enterprise-fixture");
        QVERIFY(service.registerService("org.freedesktop.NetworkManager"));
        QVERIFY(service.registerVirtualObject(fake.base, &fake, QDBusConnection::SubPath));
        NetworkSettings settings(nullptr, bus);
        settings.refresh();
        QTRY_VERIFY(!settings.busy());
        QTemporaryFile ca;
        QVERIFY(ca.open());
        ca.write("fixture certificate");
        ca.flush();
        QVariantMap d{{"name", "Enterprise"},
                      {"type", "802-11-wireless"},
                      {"ssid", "Office"},
                      {"enterprise", true},
                      {"eap", "peap"},
                      {"identity", "alice"},
                      {"phase2Auth", "mschapv2"},
                      {"caCert", ca.fileName()},
                      {"domainSuffixMatch", "radius.example.org"},
                      {"eapPassword", "fixture-secret"}};
        settings.saveProfile("", d);
        QTRY_VERIFY(!settings.busy());
        QCOMPARE(fake.written.value("connection").value("type").toString(), QString("802-11-wireless"));
        QCOMPARE(fake.written.value("802-1x").value("eap").toStringList(), QStringList{"peap"});
        QCOMPARE(fake.written.value("802-1x").value("password").toString(), QString("fixture-secret"));
        QCOMPARE(fake.written.value("802-1x").value("ca-cert").toByteArray(),
                 QByteArray("file://") + ca.fileName().toUtf8() + QByteArray(1, '\0'));
        QVERIFY(!fake.written.value("connection").value("permissions").toStringList().isEmpty());
        int before = fake.writes;
        d["caCert"] = "";
        settings.saveProfile("", d);
        QCOMPARE(fake.writes, before);
        d["caCert"] = ca.fileName();
        d["domainSuffixMatch"] = "";
        settings.saveProfile("", d);
        QCOMPARE(fake.writes, before);
        d["domainSuffixMatch"] = "radius.example.org";
        d["eap"] = "tls";
        settings.saveProfile("", d);
        QCOMPARE(fake.writes, before);
        d["clientCert"] = ca.fileName();
        d["privateKey"] = ca.fileName();
        d["type"] = "802-3-ethernet";
        settings.saveProfile("", d);
        QTRY_VERIFY(!settings.busy());
        QCOMPARE(fake.written.value("802-1x").value("eap").toStringList(), QStringList{"tls"});
        QVERIFY(!fake.written.value("802-1x").contains("password"));
        QVERIFY(!fake.written.contains("802-11-wireless-security"));
        d["eap"] = "ttls";
        d["phase2Auth"] = "pap";
        d["eapPassword"] = "";
        settings.saveProfile("", d);
        QTRY_VERIFY(!settings.busy());
        QCOMPARE(fake.written.value("802-1x").value("phase2-auth").toString(), QString("pap"));
        fake.saved = fake.written;
        settings.refresh();
        QTRY_VERIFY(!settings.busy());
        auto edited = settings.profileDraft(fake.base + "/Settings/1");
        QVERIFY(!edited.contains("eapPassword"));
        QVERIFY(!edited.contains("privateKeyPassword"));
        QCOMPARE(edited.value("caCert").toString(), ca.fileName());
        edited["identity"] = "updated-user";
        settings.saveProfile(fake.base + "/Settings/1", edited);
        QTRY_VERIFY(!settings.busy());
        QCOMPARE(fake.written.value("802-1x").value("password").toString(), QString("existing-eap-secret"));
        QCOMPARE(fake.written.value("802-1x").value("private-key-password").toString(),
                 QString("existing-key-secret"));
        edited["eapPassword"] = "replacement";
        settings.saveProfile(fake.base + "/Settings/1", edited);
        QTRY_VERIFY(!settings.busy());
        QCOMPARE(fake.written.value("802-1x").value("password").toString(), QString("replacement"));
        before = fake.writes;
        d["phase2Auth"] = "tls";
        settings.saveProfile("", d);
        QCOMPARE(fake.writes, before);
        d["phase2Auth"] = "pap";
        d["caCert"] = "/does-not-exist.pem";
        settings.saveProfile("", d);
        QCOMPARE(fake.writes, before);
        fake.saved["802-1x"]["eap"] = QStringList{"tls"};
        fake.requestedSetting = "802-1x";
        fake.requestedHints.clear();
        settings.refresh();
        QTRY_VERIFY(!settings.busy());
        settings.activate(fake.base + "/Settings/1");
        QTRY_VERIFY(!settings.secretPrompt().isEmpty());
        QCOMPARE(settings.secretPrompt().value("fields").toList().first().toMap().value("key").toString(),
                 QString("private-key-password"));
        settings.cancelSecrets();
        service.unregisterObject(fake.base, QDBusConnection::UnregisterTree);
        service.unregisterService("org.freedesktop.NetworkManager");
    }
    void isolatedProfiles() {
        auto bus = QDBusConnection::sessionBus();
        QVERIFY(bus.isConnected());
        FakeNetwork fake;
        auto fakeBus = QDBusConnection::connectToBus(QDBusConnection::SessionBus, "fake-network-settings");
        QVERIFY(fakeBus.registerService("org.freedesktop.NetworkManager"));
        QVERIFY(fakeBus.registerVirtualObject("/org/freedesktop/NetworkManager", &fake,
                                              QDBusConnection::SubPath));
        NetworkSettings settings(nullptr, bus);
        QVERIFY(settings.devices().isEmpty());
        settings.refresh();
        QTRY_VERIFY(!settings.busy());
        QVERIFY(settings.available());
        QCOMPARE(settings.devices().size(), 1);
        QCOMPARE(settings.networks().size(), 1);
        QCOMPARE(settings.profiles().size(), 1);
        settings.connectWifi("/invalid", "password123");
        QCOMPARE(fake.writes, 0);
        settings.connectWifi(fake.base + "/AccessPoint/1", "short");
        QCOMPARE(fake.writes, 0);
        settings.connectWifi(fake.base + "/AccessPoint/1", "secret-fixture-123");
        QTRY_VERIFY(!settings.busy());
        QCOMPARE(fake.written.value("802-11-wireless-security").value("psk").toString(),
                 QString("secret-fixture-123"));
        QVERIFY(!fake.written.value("connection").value("permissions").toStringList().isEmpty());
        QVERIFY(!settings.networks().first().toMap().contains("psk"));
        auto draft = settings.profileDraft(fake.base + "/Settings/1");
        draft["ipv4Method"] = "manual";
        draft["ipv4Addresses"] = "10.1.2.3/24";
        draft["ipv4Gateway"] = "10.1.2.1";
        draft["ipv4ManualDns"] = true;
        draft["ipv4Dns"] = "1.1.1.1";
        settings.saveProfile(fake.base + "/Settings/1", draft);
        QTRY_VERIFY(!settings.busy());
        QCOMPARE(fake.written.value("ipv4").value("method").toString(), QString("manual"));
        QCOMPARE(qdbus_cast<NetworkAddresses>(fake.written.value("ipv4").value("address-data")).size(), 1);
        const int before = fake.writes;
        draft["ipv4Addresses"] = "garbage";
        settings.saveProfile(fake.base + "/Settings/1", draft);
        QCOMPARE(fake.writes, before);
        fake.denied = true;
        draft["ipv4Addresses"] = "10.1.2.4/24";
        settings.saveProfile(fake.base + "/Settings/1", draft);
        QTRY_VERIFY(!settings.busy());
        QCOMPARE(settings.error(), QString("Fixture denied"));
        fake.denied = false;
        fake.saved["vpn"] = {{"service-type", "org.freedesktop.NetworkManager.openvpn"},
                             {"data", QVariant::fromValue(QMap<QString, QString>{{"username", "before"}})}};
        settings.refresh();
        QTRY_VERIFY(!settings.busy());
        auto vpnDraft = settings.profileDraft(fake.base + "/Settings/1");
        vpnDraft["vpnUsername"] = "fixture-login";
        vpnDraft["vpnPassword"] = "new-secret";
        settings.saveProfile(fake.base + "/Settings/1", vpnDraft);
        QTRY_VERIFY(!settings.busy());
        const auto secrets = qdbus_cast<QMap<QString, QString>>(fake.written.value("vpn").value("secrets"));
        QCOMPARE(secrets.value("password"), QString("new-secret"));
        QCOMPARE(secrets.value("cert-pass"), QString("keep-cert"));
        settings.activate(fake.base + "/Settings/1");
        QTRY_VERIFY(!settings.secretPrompt().isEmpty());
        QVERIFY(fake.receivedSecret.isEmpty());
        auto intruder = QDBusConnection::connectToBus(QDBusConnection::SessionBus, "network-agent-intruder");
        auto request =
            QDBusMessage::createMethodCall(bus.baseService(), "/org/harbor/NetworkAgent",
                                           "org.freedesktop.NetworkManager.SecretAgent", "GetSecrets");
        request.setArguments({QVariant::fromValue(fake.saved),
                              QVariant::fromValue(QDBusObjectPath(fake.base + "/Settings/1")), QString("vpn"),
                              QStringList{"password"}, uint(1)});
        QDBusPendingCallWatcher deniedRequest(intruder.asyncCall(request, 1000));
        QTRY_VERIFY(deniedRequest.isFinished());
        QCOMPARE(deniedRequest.reply().type(), QDBusMessage::ErrorMessage);
        settings.respondSecrets(true, {{"password", "interactive-secret"}});
        QTRY_COMPARE(fake.receivedSecret, QString("interactive-secret"));
        QVERIFY(settings.secretPrompt().isEmpty());
        QDBusPendingCallWatcher secondDenied(intruder.asyncCall(request, 1000));
        QTRY_VERIFY(secondDenied.isFinished());
        QCOMPARE(secondDenied.reply().type(), QDBusMessage::ErrorMessage);
        QVERIFY(settings.secretPrompt().isEmpty());
        fakeBus.unregisterObject(fake.base, QDBusConnection::UnregisterTree);
        fakeBus.unregisterService("org.freedesktop.NetworkManager");
    }
};
QTEST_GUILESS_MAIN(NetworkTest)
#include "network_settings_test.moc"
