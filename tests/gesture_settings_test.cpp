#include <QtTest>
#include <QDBusVirtualObject>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QTemporaryDir>
#include <QFile>
#include <QRegularExpression>
#include "core/GestureSettings.h"
class GestureFake : public QDBusVirtualObject {
public:
    bool loaded = false, reject = false, ready = true;
    QString source;
    int mutations = 0;
    QString introspect(const QString&) const override { return {}; }
    bool handleMessage(const QDBusMessage& message, const QDBusConnection& bus) override {
        if (message.member() == "isScriptLoaded") {
            bus.send(message.createReply(QVariantList{loaded}));
            return true;
        }
        ++mutations;
        if (reject) {
            bus.send(message.createErrorReply("org.freedesktop.DBus.Error.Failed", "Test KWin failure"));
            return true;
        }
        if (message.member() == "unloadScript") {
            loaded = false;
            bus.send(message.createReply(QVariantList{true}));
            return true;
        }
        if (message.member() == "loadDeclarativeScript") {
            QFile f(message.arguments()[0].toString());
            if (!f.open(QIODevice::ReadOnly)) {
                bus.send(message.createErrorReply("org.freedesktop.DBus.Error.Failed", "Script missing"));
                return true;
            }
            source = QString::fromUtf8(f.readAll());
            loaded = true;
            bus.send(message.createReply(QVariantList{7}));
            return true;
        }
        if (message.member() == "run") {
            bus.send(message.createReply());
            if (ready) {
                auto line = source.split('\n').value(3);
                QRegularExpression service("service: \"([^\"]+)\""), path("path: \"([^\"]+)\""),
                    token("arguments: \\[\"([^\"]+)\"\\]");
                auto callback = QDBusMessage::createMethodCall(service.match(line).captured(1),
                                                               path.match(line).captured(1),
                                                               "org.harbor.Gestures", "Ready");
                callback << token.match(line).captured(1);
                bus.asyncCall(callback);
            }
            return true;
        }
        return false;
    }
};
class GestureTest : public QObject {
    Q_OBJECT
private slots:
    void closedActions() {
        QVERIFY(!GestureSettings::validMappings({{"3-Up", "console.log(1)"}}));
        QVERIFY(!GestureSettings::validMappings({{"9-Up", "launcher"}}));
        auto code =
            GestureSettings::script({{"3-Up", "launcher"}, {"4-Down", "settings"}}, ":1.5", "/test", "token");
        QVERIFY(code.contains("fingerCount: 3"));
        QVERIFY(code.contains("SwipeGestureHandler.Direction.Down"));
        QVERIFY(code.contains("org.harbor.Shell"));
        QVERIFY(GestureSettings::script({{"3-Up", "bad\"code"}}, "", "", "").isEmpty());
    }
    void privateBusApplyAndRestore() {
        auto bus = QDBusConnection::sessionBus();
        QVERIFY(!bus.interface()->isServiceRegistered("org.kde.KWin").value());
        GestureFake fake;
        QVERIFY(bus.registerService("org.kde.KWin"));
        QVERIFY(bus.registerVirtualObject("/Scripting", &fake, QDBusConnection::SubPath));
        QTemporaryDir temp;
        {
            GestureSettings settings(nullptr, bus, temp.path());
            QVERIFY(!settings.enabled());
            settings.refresh();
            QTRY_VERIFY(!settings.busy());
            QVERIFY(settings.available());
            settings.apply({{"3-Up", "launcher"}}, true);
            QTRY_VERIFY(!settings.busy());
            QVERIFY2(settings.error().isEmpty(), qPrintable(settings.error()));
            QVERIFY(settings.active());
            QVERIFY(settings.enabled());
            QVERIFY(QFile::exists(temp.path() + "/settings.json"));
            int calls = fake.mutations;
            settings.apply({{"3-Up", "arbitrary-script"}}, true);
            QCOMPARE(fake.mutations, calls);
            settings.apply(settings.mappings(), false);
            QTRY_VERIFY(!settings.busy());
            QVERIFY(!settings.enabled());
            QVERIFY(!fake.loaded);
            fake.ready = false;
            settings.apply({{"4-Down", "settings"}}, true);
            QTRY_VERIFY_WITH_TIMEOUT(!settings.busy(), 7000);
            QVERIFY(settings.error().contains("initialize"));
            QTRY_VERIFY(!fake.loaded);
            QVERIFY(!settings.enabled());
            fake.ready = true;
            settings.apply({{"4-Down", "settings"}}, true);
            QTRY_VERIFY(!settings.busy());
            QVERIFY(settings.enabled());
            fake.reject = true;
            settings.apply(settings.mappings(), false);
            QTRY_VERIFY(!settings.busy());
            QVERIFY(settings.error().contains("Test KWin failure"));
            QVERIFY(settings.enabled());
            fake.reject = false;
        }
        {
            GestureSettings restored(nullptr, bus, temp.path());
            restored.restore();
            QTRY_VERIFY(!restored.busy());
            QVERIFY(restored.active());
            QVERIFY(restored.enabled());
            restored.apply(restored.mappings(), false);
            QTRY_VERIFY(!restored.busy());
        }
        bus.unregisterObject("/Scripting");
        bus.unregisterService("org.kde.KWin");
    }
};
QTEST_GUILESS_MAIN(GestureTest)
#include "gesture_settings_test.moc"
