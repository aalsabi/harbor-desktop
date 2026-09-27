#include <QtTest>
#include <QQuickView>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQmlContext>
#include <QQmlEngine>
#include <QTemporaryDir>
#include "core/Preferences.h"
#include "core/Translation.h"

// Stands in for NetworkSettings: serves a fixed profile draft and records what the page sends back.
class NetworkFixture : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap secretPrompt MEMBER secretPrompt NOTIFY changed)
    Q_PROPERTY(QVariantList devices MEMBER empty NOTIFY changed)
    Q_PROPERTY(QVariantList networks MEMBER empty NOTIFY changed)
    Q_PROPERTY(QVariantList profiles MEMBER empty NOTIFY changed)
    Q_PROPERTY(bool busy MEMBER busy NOTIFY changed)
    Q_PROPERTY(bool available MEMBER available NOTIFY changed)
    Q_PROPERTY(bool wirelessEnabled MEMBER wirelessEnabled NOTIFY changed)
    Q_PROPERTY(QString error MEMBER error NOTIFY changed)
public:
    QVariantMap secretPrompt;
    QVariantList empty;
    bool busy = false, available = true, wirelessEnabled = true;
    QString error;
    QVariantMap nextDraft;
    QString savedPath;
    QVariantMap savedDraft;
    int saves = 0;
    Q_INVOKABLE void respondSecrets(bool, const QVariantMap&) {}
    Q_INVOKABLE void cancelSecrets() {}
    Q_INVOKABLE void setActive(bool) {}
    Q_INVOKABLE void refresh() {}
    Q_INVOKABLE void setWirelessEnabled(bool) {}
    Q_INVOKABLE void scan() {}
    Q_INVOKABLE void connectWifi(const QString&, const QString&) {}
    Q_INVOKABLE void activate(const QString&) {}
    Q_INVOKABLE void disconnectDevice(const QString&) {}
    Q_INVOKABLE QVariantMap profileDraft(const QString& path) const {
        return path.isEmpty() ? QVariantMap{{"type", "802-3-ethernet"}} : nextDraft;
    }
    Q_INVOKABLE void saveProfile(const QString& path, const QVariantMap& draft) {
        savedPath = path;
        savedDraft = draft;
        ++saves;
    }
    Q_INVOKABLE void importVpn(const QString&, const QString&) {}
signals:
    void changed();
    void profileSaved(const QString& path);
};

static QQuickItem* named(QQuickItem* root, const QString& name) {
    if (root->objectName() == name)
        return root;
    for (auto child : root->childItems())
        if (auto match = named(child, name))
            return match;
    return nullptr;
}

class NetworkUiTest : public QObject {
    Q_OBJECT
    QTemporaryDir temp;
    Preferences* prefs = nullptr;
    NetworkFixture* network = nullptr;
    QQuickView* view = nullptr;
    QStringList warnings;

    QQuickItem* page() const { return view->rootObject(); }
    QQuickItem* item(const QString& name) const {
        auto found = named(page(), name);
        if (!found)
            qWarning("missing item %s", qPrintable(name));
        return found;
    }
    // Simulates a user edit: set the text, then emit textEdited as typing would.
    void type(const QString& name, const QString& text) {
        auto field = item(name);
        QVERIFY(field);
        field->setProperty("text", text);
        QVERIFY(QMetaObject::invokeMethod(field, "textEdited"));
    }
    void choose(const QString& name, int index) {
        auto combo = item(name);
        QVERIFY(combo);
        combo->setProperty("currentIndex", index);
        QVERIFY(QMetaObject::invokeMethod(combo, "activated", Q_ARG(int, index)));
    }
    void click(const QString& name) {
        auto button = item(name);
        QVERIFY(button);
        QVERIFY(button->isEnabled());
        QVERIFY(QMetaObject::invokeMethod(button, "clicked"));
    }
    QVariantMap draft() const { return page()->property("draft").toMap(); }

private slots:
    void initTestCase() { QQuickStyle::setStyle("Basic"); }
    void init() {
        warnings.clear();
        prefs = new Preferences(temp.filePath("prefs.ini"));
        prefs->setLanguage("en");
        network = new NetworkFixture;
        network->nextDraft = {{"name", "Office LAN"},
                              {"type", "802-3-ethernet"},
                              {"ipv4Method", "auto"},
                              {"ipv6Method", "auto"}};
        view = new QQuickView;
        connect(view->engine(), &QQmlEngine::warnings, this, [this](const QList<QQmlError>& errors) {
            for (const auto& e : errors)
                warnings << e.toString();
        });
        view->rootContext()->setContextProperty("Prefs", prefs);
        view->rootContext()->setContextProperty("NetworkSettings", network);
        view->setResizeMode(QQuickView::SizeRootObjectToView);
        view->resize(700, 900);
        view->setSource(QUrl("qrc:/qml/NetworkPage.qml"));
        QCOMPARE(view->status(), QQuickView::Ready);
        view->show();
    }
    void cleanup() {
        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.join('\n')));
        delete view;
        delete network;
        delete prefs;
    }

    void editorHiddenUntilEditing() {
        auto editor = item("connectionEditor");
        QVERIFY(editor);
        QVERIFY(!editor->isVisible());
        QVERIFY(QMetaObject::invokeMethod(page(), "edit", Q_ARG(QVariant, QString("/profile/1"))));
        QVERIFY(editor->isVisible());
    }

    void editingAProfileFillsTheFields() {
        QVERIFY(QMetaObject::invokeMethod(page(), "edit", Q_ARG(QVariant, QString("/profile/1"))));
        QCOMPARE(item("connectionName")->property("text").toString(), QString("Office LAN"));
        QCOMPARE(item("ipv4Method")->property("currentIndex").toInt(), 0);
        QVERIFY(!item("ipv4Addresses")->isVisible());
    }

    void manualAddressingIsSavedWithTheProfile() {
        QVERIFY(QMetaObject::invokeMethod(page(), "edit", Q_ARG(QVariant, QString("/profile/1"))));
        type("connectionName", "Office LAN 2");
        choose("ipv4Method", 1);
        QVERIFY(item("ipv4Addresses")->isVisible());
        QVERIFY(!item("ipv6Addresses")->isVisible());
        type("ipv4Addresses", "192.168.1.20/24");
        click("saveConnection");
        QCOMPARE(network->saves, 1);
        QCOMPARE(network->savedPath, QString("/profile/1"));
        QCOMPARE(network->savedDraft.value("name").toString(), QString("Office LAN 2"));
        QCOMPARE(network->savedDraft.value("ipv4Method").toString(), QString("manual"));
        QCOMPARE(network->savedDraft.value("ipv4Addresses").toString(), QString("192.168.1.20/24"));
        QCOMPARE(network->savedDraft.value("ipv6Method").toString(), QString("auto"));
        // Secrets are cleared from the draft once they have been handed to NetworkManager.
        QCOMPARE(draft().value("vpnPassword").toString(), QString());
        QCOMPARE(draft().value("eapPassword").toString(), QString());
    }

    void savingIsDisabledWhileBusy() {
        QVERIFY(QMetaObject::invokeMethod(page(), "edit", Q_ARG(QVariant, QString("/profile/1"))));
        network->busy = true;
        emit network->changed();
        QVERIFY(!item("saveConnection")->isEnabled());
    }

    void closeDiscardsTheDraft() {
        QVERIFY(QMetaObject::invokeMethod(page(), "edit", Q_ARG(QVariant, QString("/profile/1"))));
        type("connectionName", "Unsaved");
        click("closeConnection");
        QVERIFY(!item("connectionEditor")->isVisible());
        QVERIFY(draft().isEmpty());
        QCOMPARE(network->saves, 0);
    }

    void savedProfileClosesTheEditor() {
        QVERIFY(QMetaObject::invokeMethod(page(), "edit", Q_ARG(QVariant, QString())));
        emit network->profileSaved("/profile/2");
        QVERIFY(!item("connectionEditor")->isVisible());
        QCOMPARE(page()->property("editingPath").toString(), QString("/profile/2"));
    }

    void newEnterpriseNetworkShowsAuthentication() {
        QVERIFY(QMetaObject::invokeMethod(page(), "enterprise", Q_ARG(QVariant, QString("Corp"))));
        QVERIFY(item("connectionEditor")->isVisible());
        QVERIFY(item("eapMethod")->isVisible());
        QCOMPARE(item("eapMethod")->property("currentIndex").toInt(), 0);
        QCOMPARE(draft().value("ssid").toString(), QString("Corp"));
        QCOMPARE(draft().value("eap").toString(), QString("peap"));
        choose("eapMethod", 2);
        QCOMPARE(draft().value("eap").toString(), QString("tls"));
    }

    void editorFitsInArabic() {
        Translation translation(*prefs);
        connect(&translation, &Translation::changed, view->engine(), &QQmlEngine::retranslate);
        prefs->setLanguage("ar");
        view->resize(500, 900);
        QVERIFY(QMetaObject::invokeMethod(page(), "enterprise", Q_ARG(QVariant, QString("Corp"))));
        QTest::qWait(20);
        const QRectF window(0, 0, view->width(), view->height());
        for (auto name : {"connectionName", "eapMethod", "ipv4Method", "saveConnection", "closeConnection"}) {
            auto control = item(name);
            QVERIFY(control);
            auto bounds = control->mapRectToScene(control->boundingRect());
            QVERIFY2(bounds.left() >= window.left() && bounds.right() <= window.right(), name);
        }
        prefs->setLanguage("en");
    }
};
QTEST_MAIN(NetworkUiTest)
#include "network_ui_test.moc"
