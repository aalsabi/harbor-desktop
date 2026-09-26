#include <QtTest>
#include <QQuickView>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQmlContext>
#include <QTemporaryDir>
#include "core/Preferences.h"
class InputMock : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString activeLabel MEMBER activeLabel NOTIFY changed)
    Q_PROPERTY(QString activeName MEMBER activeName NOTIFY changed)
    Q_PROPERTY(QString activeTitle MEMBER activeTitle NOTIFY changed)
    Q_PROPERTY(bool available MEMBER available NOTIFY changed)
    Q_PROPERTY(int activeIndex MEMBER activeIndex NOTIFY changed)
    Q_PROPERTY(QVariantList activeLayouts MEMBER activeLayouts NOTIFY changed)
    Q_PROPERTY(QVariantList roots MEMBER empty NOTIFY changed)
    Q_PROPERTY(QVariantList items MEMBER empty NOTIFY changed)
    Q_PROPERTY(QString message MEMBER message NOTIFY changed)
public:
    QString activeLabel = "EN", activeName = "English (US)", activeTitle = "Harbor", message, page;
    bool available = true, succeed = false;
    int activeIndex = 0, selected = -1, dismissed = 0, cycles = 0;
    QVariantList empty,
        activeLayouts{QVariantMap{{"index", 0}, {"name", "English (US)"}, {"label", "EN"}},
                      QVariantMap{{"index", 1}, {"name", "Arabic"}, {"label", QString::fromUtf8("ع")}}};
    Q_INVOKABLE void open(QString p) { page = p; }
    Q_INVOKABLE void dismiss() { ++dismissed; }
    Q_INVOKABLE void switchNext() { ++cycles; }
    Q_INVOKABLE void refreshActive() {}
    Q_INVOKABLE void selectLayout(int index) {
        selected = index;
        emit selectionFinished(succeed);
    }
signals:
    void changed();
    void selectionFinished(bool success);
};
QQuickItem* inputItem(QQuickItem* root, QString name) {
    if (root->objectName() == name)
        return root;
    for (auto c : root->childItems())
        if (auto found = inputItem(c, name))
            return found;
    return nullptr;
}
class InputMenuTest : public QObject {
    Q_OBJECT
private slots:
    void clickOpensSelectableMenu() {
        QQuickStyle::setStyle("Basic");
        QTemporaryDir temp;
        Preferences prefs(temp.filePath("prefs.ini"));
        InputMock mock;
        QQuickView view;
        auto ctx = view.rootContext();
        ctx->setContextProperty("Prefs", &prefs);
        for (auto name : {"Keyboard", "UI", "Windows", "Tray", "GlobalMenu", "Notifications"})
            ctx->setContextProperty(name, &mock);
        view.setResizeMode(QQuickView::SizeRootObjectToView);
        view.resize(1440, 38);
        view.setSource(QUrl("qrc:/qml/MenuBar.qml"));
        QVERIFY(view.status() == QQuickView::Ready);
        view.show();
        QTest::qWait(30);
        auto button = inputItem(view.rootObject(), "keyboard-indicator");
        QVERIFY(button);
        QTest::mouseClick(&view, Qt::LeftButton, Qt::NoModifier,
                          button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint());
        QCOMPARE(mock.page, QString("input"));
        QCOMPARE(mock.cycles, 0);
    }
    void choosingLanguageClosesOnlyOnSuccess() {
        QTemporaryDir temp;
        Preferences prefs(temp.filePath("prefs.ini"));
        InputMock mock;
        QQuickView view;
        auto ctx = view.rootContext();
        ctx->setContextProperty("Prefs", &prefs);
        ctx->setContextProperty("Keyboard", &mock);
        ctx->setContextProperty("UI", &mock);
        view.setResizeMode(QQuickView::SizeRootObjectToView);
        view.resize(320, 300);
        view.setSource(QUrl("qrc:/qml/InputMenu.qml"));
        QVERIFY(view.status() == QQuickView::Ready);
        view.show();
        QTest::qWait(30);
        if (qEnvironmentVariableIsSet("HARBOR_TEST_SCREENSHOTS"))
            view.grabWindow().save("/tmp/harbor-input-menu.png");
        auto button = inputItem(view.rootObject(), "input-source-1");
        QVERIFY(button);
        auto point = button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint();
        QTest::mouseClick(&view, Qt::LeftButton, Qt::NoModifier, point);
        QCOMPARE(mock.selected, 1);
        QCOMPARE(mock.dismissed, 0);
        mock.succeed = true;
        QTest::mouseClick(&view, Qt::LeftButton, Qt::NoModifier, point);
        QCOMPARE(mock.dismissed, 1);
    }
};
QTEST_MAIN(InputMenuTest)
#include "input_menu_test.moc"
