#include <QtTest>
#include <QQuickView>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQmlContext>
#include <QTemporaryDir>
#include "core/Preferences.h"
class ControlMock : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap state MEMBER state NOTIFY changed)
    Q_PROPERTY(bool busy MEMBER busy NOTIFY changed)
    Q_PROPERTY(QString message MEMBER message NOTIFY changed)
public:
    QVariantMap state;
    bool busy = false;
    QString message, command, page;
    QVariant value;
    int logouts = 0;
    Q_INVOKABLE void action(QString name, QVariant v = {}) {
        command = name;
        value = v;
    }
    Q_INVOKABLE void refresh() {}
    Q_INVOKABLE void open(QString p) { page = p; }
    Q_INVOKABLE void dismiss() {}
    Q_INVOKABLE void logout() { ++logouts; }
    Q_INVOKABLE void switchNext() { command = "keyboard"; }
signals:
    void changed();
};
QQuickItem* controlItem(QQuickItem* item, QString name) {
    if (item->objectName() == name)
        return item;
    for (auto child : item->childItems())
        if (auto result = controlItem(child, name))
            return result;
    return nullptr;
}
QQuickItem* brightnessItem(QQuickItem* item) {
    if (item->property("to").toInt() == 100 && item->property("from").toInt() == 5)
        return item;
    for (auto child : item->childItems())
        if (auto result = brightnessItem(child))
            return result;
    return nullptr;
}
class ControlUITest : public QObject {
    Q_OBJECT
private slots:
    void liveValuesAndConfirmation() {
        QQuickStyle::setStyle("Basic");
        QTemporaryDir temp;
        Preferences prefs(temp.filePath("prefs.ini"));
        ControlMock system, keyboard, ui;
        system.state = {{"brightnessAvailable", true}, {"brightnessPercent", 37},
                        {"wifiAvailable", true},       {"wifi", "disabled"},
                        {"bluetoothAvailable", true},  {"volumeAvailable", true},
                        {"outputVolume", .42},         {"harborSession", true},
                        {"canReboot", true},           {"canPowerOff", true}};
        QQuickView view;
        view.rootContext()->setContextProperty("Prefs", &prefs);
        view.rootContext()->setContextProperty("System", &system);
        view.rootContext()->setContextProperty("Keyboard", &keyboard);
        view.rootContext()->setContextProperty("UI", &ui);
        view.setResizeMode(QQuickView::SizeRootObjectToView);
        view.resize(400, 900);
        auto old = qEnvironmentVariable("HARBOR_CONTROL_QML");
        view.setSource(old.isEmpty() ? QUrl("qrc:/qml/ControlCenter.qml") : QUrl::fromLocalFile(old));
        QVERIFY(view.status() == QQuickView::Ready);
        view.show();
        QTest::qWait(30);
        auto brightness = brightnessItem(view.rootObject());
        QVERIFY(brightness);
        QCOMPARE(brightness->property("value").toDouble(), 37.);
        brightness->setProperty("value", 73.);
        QMetaObject::invokeMethod(brightness, "moved");
        emit system.changed();
        QTest::qWait(230);
        QCOMPARE(system.command, QString("brightness"));
        QCOMPARE(system.value.toInt(), 73);
        auto wifi = controlItem(view.rootObject(), "control-wifi");
        QVERIFY(wifi);
        QTest::mouseClick(&view, Qt::LeftButton, Qt::NoModifier, wifi->mapToScene(QPointF(20, 20)).toPoint());
        QCOMPARE(system.command, QString("wifi"));
        QVERIFY(system.value.toBool());
        auto logout = controlItem(view.rootObject(), "session-logout");
        QVERIFY(logout);
        QMetaObject::invokeMethod(logout, "clicked");
        QCOMPARE(ui.logouts, 0);
        auto dialog = view.rootObject()->findChild<QObject*>("session-confirm");
        QVERIFY(dialog);
        QVERIFY(dialog->property("visible").toBool());
        QMetaObject::invokeMethod(dialog, "reject");
        QCOMPARE(ui.logouts, 0);
        QMetaObject::invokeMethod(logout, "clicked");
        QMetaObject::invokeMethod(dialog, "accept");
        QCOMPARE(ui.logouts, 1);
        system.state["brightnessPercent"] = 61;
        emit system.changed();
        QCOMPARE(brightness->property("value").toDouble(), 61.);
    }
};
QTEST_MAIN(ControlUITest)
#include "control_ui_test.moc"
