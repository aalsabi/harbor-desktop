#include <QtTest>
#include <QQuickView>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQmlContext>
#include <QTemporaryDir>
#include "core/Preferences.h"
class DockMock : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList entries MEMBER entries NOTIFY changed)
    Q_PROPERTY(QVariantList windows MEMBER windows NOTIFY changed)
    Q_PROPERTY(QStringList pendingLaunches MEMBER pending NOTIFY changed)
public:
    QVariantList entries{
        QVariantMap{{"id", "console.desktop"}, {"name", "Console"}, {"icon", "utilities-terminal"}}},
        windows;
    QStringList pending;
    int launches = 0;
    Q_INVOKABLE QString desktopForTool(QString) { return {}; }
    Q_INVOKABLE QString iconForAppId(QString) { return "app"; }
    Q_INVOKABLE bool launch(QString id) {
        ++launches;
        pending << id;
        emit changed();
        return true;
    }
    Q_INVOKABLE void activate(QString) {}
    Q_INVOKABLE void open(QString) {}
    Q_INVOKABLE void openTool(QString) {}
signals:
    void changed();
};
QQuickItem* dockItem(QQuickItem* root, QString name) {
    if (root->objectName() == name)
        return root;
    for (auto child : root->childItems())
        if (auto found = dockItem(child, name))
            return found;
    return nullptr;
}
class DockLaunchTest : public QObject {
    Q_OBJECT
private slots:
    void repeatedPointerClicksWhileStarting() {
        QQuickStyle::setStyle("Basic");
        QTemporaryDir temp;
        Preferences prefs(temp.filePath("prefs.ini"));
        prefs.setPins({"console.desktop"});
        DockMock mock;
        QQuickView view;
        auto ctx = view.rootContext();
        ctx->setContextProperty("Prefs", &prefs);
        for (auto key : {"Apps", "Windows", "System", "UI"})
            ctx->setContextProperty(key, &mock);
        view.setResizeMode(QQuickView::SizeRootObjectToView);
        view.resize(720, 78);
        view.setSource(QUrl("qrc:/qml/Dock.qml"));
        QVERIFY(view.status() == QQuickView::Ready);
        view.show();
        QTest::qWait(30);
        auto button = dockItem(view.rootObject(), "dock-app-console.desktop");
        QVERIFY(button);
        auto point = button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint();
        for (int i = 0; i < 8; ++i)
            QTest::mouseClick(&view, Qt::LeftButton, Qt::NoModifier, point);
        QCOMPARE(mock.launches, 1);
        QVERIFY(button->property("launching").toBool());
        QVERIFY(!button->isEnabled());
    }
};
QTEST_MAIN(DockLaunchTest)
#include "dock_launch_test.moc"
