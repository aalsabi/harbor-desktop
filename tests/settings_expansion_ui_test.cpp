#include <QtTest>
#include <QQuickView>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQmlContext>
#include <QQmlEngine>
#include <QTemporaryDir>
#include "core/Preferences.h"
#include "core/Translation.h"
class PageFixture : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList applications MEMBER extra NOTIFY changed)
    Q_PROPERTY(QVariantList associations MEMBER extra NOTIFY changed)
    Q_PROPERTY(QVariantMap summary MEMBER state NOTIFY changed)
    Q_PROPERTY(QVariantList discovered MEMBER extra NOTIFY changed)
    Q_PROPERTY(QVariantMap selectedDevice MEMBER state NOTIFY changed)
    Q_PROPERTY(QVariantList packages MEMBER extra NOTIFY changed)
    Q_PROPERTY(QVariantList preview MEMBER extra NOTIFY changed)
    Q_PROPERTY(bool checked MEMBER busy NOTIFY changed)
    Q_PROPERTY(bool readyToInstall MEMBER busy NOTIFY changed)
    Q_PROPERTY(int percentage MEMBER percentage NOTIFY changed)
    Q_PROPERTY(QString status MEMBER message NOTIFY changed)
    Q_PROPERTY(QString restart MEMBER message NOTIFY changed)
    Q_PROPERTY(QVariantList drivers MEMBER extra NOTIFY changed)
    Q_PROPERTY(QVariantList options MEMBER extra NOTIFY changed)
    Q_PROPERTY(QString selectedPrinter MEMBER selected NOTIFY changed)
    Q_PROPERTY(QVariantMap state MEMBER state NOTIFY changed)
    Q_PROPERTY(QVariantList users MEMBER users NOTIFY changed)
    Q_PROPERTY(QVariantList devices MEMBER devices NOTIFY changed)
    Q_PROPERTY(bool analyzing MEMBER busy NOTIFY changed)
    Q_PROPERTY(QVariantList categories MEMBER categories NOTIFY changed)
    Q_PROPERTY(QVariantList largestFiles MEMBER categories NOTIFY changed)
    Q_PROPERTY(QString analysisMessage MEMBER message NOTIFY changed)
    Q_PROPERTY(QString homePath MEMBER home NOTIFY changed)
    Q_PROPERTY(QVariantList roles MEMBER roles NOTIFY changed)
    Q_PROPERTY(QVariantList volumes MEMBER volumes NOTIFY changed)
    Q_PROPERTY(QStringList timezones MEMBER timezones NOTIFY changed)
    Q_PROPERTY(QString timezone MEMBER timezone NOTIFY changed)
    Q_PROPERTY(QString currentDateTime MEMBER currentDateTime NOTIFY changed)
    Q_PROPERTY(QString defaultPrinter MEMBER defaultPrinter NOTIFY changed)
    Q_PROPERTY(bool available MEMBER available NOTIFY changed)
    Q_PROPERTY(bool busy MEMBER busy NOTIFY changed)
    Q_PROPERTY(bool canNtp MEMBER canNtp NOTIFY changed)
    Q_PROPERTY(bool ntp MEMBER ntp NOTIFY changed)
    Q_PROPERTY(bool synchronized MEMBER synchronized NOTIFY changed)
    Q_PROPERTY(QString error MEMBER error NOTIFY changed)
    Q_PROPERTY(QString message MEMBER message NOTIFY changed)
public:
    int percentage = 0;
    QVariantList extra;
    QString selected;
    QVariantMap state;
    QVariantList users, devices, roles, volumes, categories;
    QString home = "/home/test";
    QStringList timezones{"Asia/Riyadh", "UTC"};
    QString timezone = "Asia/Riyadh", currentDateTime = "2026-09-24 14:15:16", defaultPrinter = "Office",
            error, message;
    bool available = true, busy = false, canNtp = true, ntp = true, synchronized = true;
    Q_INVOKABLE void request(QString, QVariantMap = {}) {}
    Q_INVOKABLE void refresh() {}
    Q_INVOKABLE void discover() {}
    Q_INVOKABLE QString suggestedFamily(QString) { return "other"; }
    Q_INVOKABLE void setActive(bool) {}
    Q_INVOKABLE void windowAction(QString) {}
    Q_INVOKABLE void setSetting(QString, QString, QVariant) {}
    Q_INVOKABLE void setNtp(bool) {}
    Q_INVOKABLE void setTimezone(QString) {}
    Q_INVOKABLE void setDateTime(QString, QString) {}
    Q_INVOKABLE bool openVolume(QString) { return true; }
    Q_INVOKABLE void setDefault(QString) {}
    Q_INVOKABLE bool setDefault(QString, QString) { return true; }
signals:
    void changed();
    void installed();
};
static QQuickItem* itemNamed(QQuickItem* root, const QString& name) {
    if (root->objectName() == name && root->isVisible())
        return root;
    for (auto c : root->childItems())
        if (auto v = itemNamed(c, name))
            return v;
    return nullptr;
}
class ExpansionUI : public QObject {
    Q_OBJECT
private slots:
    void pagesRenderAndNavigate() {
        QQuickStyle::setStyle("Basic");
        QTemporaryDir tmp;
        Preferences prefs(tmp.filePath("prefs.ini"));
        PageFixture common, pointers, printers;
        pointers.devices = {QVariantMap{{"sysName", "event1"},
                                        {"name", "USB Mouse"},
                                        {"touchpad", false},
                                        {"enabled", true},
                                        {"can_pointerAcceleration", true},
                                        {"pointerAcceleration", .2},
                                        {"can_naturalScroll", true},
                                        {"naturalScroll", false},
                                        {"can_leftHanded", true},
                                        {"leftHanded", false}},
                            QVariantMap{{"sysName", "event2"},
                                        {"name", "Built-in Touchpad"},
                                        {"touchpad", true},
                                        {"enabled", true},
                                        {"can_pointerAcceleration", true},
                                        {"pointerAcceleration", 0.},
                                        {"can_naturalScroll", true},
                                        {"naturalScroll", true},
                                        {"can_tapToClick", true},
                                        {"tapToClick", true},
                                        {"can_tapAndDrag", true},
                                        {"tapAndDrag", true},
                                        {"can_disableWhileTyping", true},
                                        {"disableWhileTyping", true}}};
        printers.devices = {QVariantMap{
            {"name", "Office"},
            {"status", "is idle. enabled"},
            {"jobs", QVariantList{QVariantMap{{"id", "Office-42"}, {"owner", "user"}, {"bytes", "2048"}}}}}};
        common.roles = {
            QVariantMap{{"key", "browser"},
                        {"types", "http, https"},
                        {"apps", QVariantList{QVariantMap{{"id", "browser.desktop"}, {"name", "Browser"}}}},
                        {"currentName", "Browser"},
                        {"mixed", false},
                        {"currentIndex", 0}}};
        common.volumes = {QVariantMap{{"mount", "/"},
                                      {"name", "System disk"},
                                      {"filesystem", "ext4"},
                                      {"readOnly", false},
                                      {"fraction", .4},
                                      {"total", qint64(500000000000)},
                                      {"used", qint64(200000000000)},
                                      {"available", qint64(290000000000)}}};
        QQuickView view;
        Translation translation(prefs);
        connect(&translation, &Translation::changed, view.engine(), &QQmlEngine::retranslate);
        QStringList warnings;
        connect(view.engine(), &QQmlEngine::warnings, this, [&](const QList<QQmlError>& es) {
            for (const auto& e : es)
                warnings << e.toString();
        });
        auto c = view.rootContext();
        c->setContextProperty("Prefs", &prefs);
        c->setContextProperty("HarborVersion", "test");
        for (const auto& name :
             {"System", "Accounts", "Keyboard", "UI", "DateTimeSettings", "StorageSettings", "DefaultApps",
              "UserSettings", "PrinterDrivers", "ApplicationStorage"})
            c->setContextProperty(name, &common);
        c->setContextProperty("PointerSettings", &pointers);
        c->setContextProperty("Printers", &printers);
        view.setResizeMode(QQuickView::SizeRootObjectToView);
        view.resize(960, 720);
        view.setSource(QUrl("qrc:/qml/Settings.qml"));
        QCOMPARE(view.status(), QQuickView::Ready);
        view.show();
        const QStringList pages{"Date & Time",          "Mouse",   "Touchpad",
                                "Default Applications", "Storage", "Printers"};
        const QStringList ids{"datetime", "mouse", "touchpad", "defaults", "storage", "printers"};
        for (int mode = 0; mode < 2; ++mode) {
            prefs.setDark(mode);
            prefs.setLanguage(mode ? "ar" : "en");
            view.resize(mode ? 740 : 960, mode ? 560 : 720);
            for (int i = 0; i < pages.size(); ++i) {
                auto search = itemNamed(view.rootObject(), "settings-search");
                QVERIFY(search);
                search->setProperty("text", pages[i]);
                QTest::qWait(20);
                auto nav = itemNamed(view.rootObject(), "settings-nav-" + ids[i]);
                QVERIFY2(nav, qPrintable(ids[i]));
                QTest::mouseClick(&view, Qt::LeftButton, Qt::NoModifier,
                                  nav->mapToScene(QPointF(nav->width() / 2, nav->height() / 2)).toPoint());
                QTest::qWait(40);
                QCOMPARE(view.rootObject()->property("section").toString(), pages[i]);
                auto loader = itemNamed(view.rootObject(), "native-settings-page");
                QVERIFY(loader);
                QCOMPARE(loader->property("status").toInt(), 1);
                auto page = qvariant_cast<QQuickItem*>(loader->property("item"));
                QVERIFY(page);
                QVERIFY(page->implicitHeight() > 0);
                QVERIFY(page->width() <= view.width() - 238);
                if (pages[i] == "Mouse" || pages[i] == "Touchpad")
                    QCOMPARE(page->property("deviceType").toString(),
                             pages[i] == "Mouse" ? QString("mouse") : QString("touchpad"));
                QVERIFY(!view.grabWindow().isNull());
                if (qEnvironmentVariableIsSet("HARBOR_TEST_SCREENSHOTS"))
                    view.grabWindow().save("/tmp/harbor-settings-" + ids[i] +
                                           (mode ? "-ar-dark" : "-en-light") + ".png");
            }
        }
        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.join('\n')));
    }
};
QTEST_MAIN(ExpansionUI)
#include "settings_expansion_ui_test.moc"
