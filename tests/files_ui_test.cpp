#include <QtTest>
#include <QQuickView>
#include <QQuickItem>
#include <QQmlContext>
#include <QTemporaryDir>
#include <QFile>
#include <QQuickStyle>
#include <QQuickImageProvider>
#include <QPixmap>
#include <QClipboard>
#include "core/Files.h"
#include "core/FilesMenu.h"
#include "core/Preferences.h"
class TestIcons : public QQuickImageProvider {
public:
    TestIcons() : QQuickImageProvider(Pixmap) {}
    QPixmap requestPixmap(const QString&, QSize*, const QSize&) override {
        QPixmap p(32, 32);
        p.fill(Qt::cyan);
        return p;
    }
};
QQuickItem* visualItem(QQuickItem* root, const QString& name) {
    if (root->objectName() == name && root->isVisible())
        return root;
    for (auto c : root->childItems())
        if (auto found = visualItem(c, name))
            return found;
    return nullptr;
}
class MockUI : public QObject {
    Q_OBJECT
public:
    QString action;
    Q_INVOKABLE void windowAction(QString value) { action = value; }
};
class FilesUI : public QObject {
    Q_OBJECT
private slots:
    void selectAndNavigate() {
        QQuickStyle::setStyle("Basic");
        QTemporaryDir d;
        QDir(d.path()).mkdir("Folder");
        QFile f(d.filePath("Note.txt"));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("preview text");
        f.close();
        Files browser;
        browser.navigate(d.path());
        Preferences prefs(d.filePath("prefs.ini"));
        MockUI ui;
        QQuickView view;
        view.rootContext()->setContextProperty("UI", &ui);
        view.engine()->addImageProvider("icons", new TestIcons);
        view.rootContext()->setContextProperty("Browser", &browser);
        view.rootContext()->setContextProperty("Prefs", &prefs);
        view.setResizeMode(QQuickView::SizeRootObjectToView);
        view.resize(1200, 760);
        view.setSource(QUrl("qrc:/qml/Files.qml"));
        QVERIFY(view.status() == QQuickView::Ready);
        view.show();
        QTest::qWait(100);
        FilesMenu exported;
        view.rootObject()->setProperty("menuExporter", QVariant::fromValue(static_cast<QObject*>(&exported)));
        exported.Event(101, "clicked", QDBusVariant(0), 0);
        QTRY_COMPARE(browser.tabs().size(), 2);
        exported.Event(302, "clicked", QDBusVariant(0), 0);
        QCOMPARE(view.rootObject()->property("viewMode").toInt(), 1);
        exported.Event(301, "clicked", QDBusVariant(0), 0);
        auto tile = visualItem(view.rootObject(), "fileTile-Note.txt");
        QVERIFY(tile);
        auto point = tile->mapToScene(QPointF(30, 30)).toPoint();
        QTest::mouseClick(&view, Qt::LeftButton, Qt::NoModifier, point);
        auto detail = view.rootObject()->property("detail");
        if (detail.metaType() == QMetaType::fromType<QJSValue>())
            detail = detail.value<QJSValue>().toVariant();
        QCOMPARE(detail.toMap()["text"].toString(), QString("preview text"));
        auto folderForSelection = visualItem(view.rootObject(), "fileTile-Folder");
        QVERIFY(folderForSelection);
        QTest::mouseClick(&view, Qt::LeftButton, Qt::ControlModifier,
                          folderForSelection->mapToScene(QPointF(30, 30)).toPoint());
        QCOMPARE(view.rootObject()->property("selected").value<QJSValue>().property("length").toInt(), 2);
        auto location = visualItem(view.rootObject(), "locationField");
        QVERIFY(location);
        location->forceActiveFocus();
        location->setProperty("text", QString());
        QGuiApplication::clipboard()->setText("a folder name");
        browser.copy({d.filePath("Note.txt")});
        auto before = browser.error();
        QTest::keyClick(&view, Qt::Key_V, Qt::ControlModifier);
        QCOMPARE(location->property("text").toString(), QString("a folder name"));
        QCOMPARE(browser.error(), before);
        QTest::keyClick(&view, Qt::Key_Space);
        QVERIFY(location->property("text").toString().endsWith(" "));
        location->setProperty("text", QString());
        QGuiApplication::clipboard()->setText("menu pasted path");
        exported.Event(203, "clicked", QDBusVariant(0), 0);
        QCOMPARE(location->property("text").toString(), QString("menu pasted path"));
        exported.Event(204, "clicked", QDBusVariant(0), 0);
        QCOMPARE(location->property("selectedText").toString(), QString("menu pasted path"));
        view.rootObject()->setProperty("viewMode", 1);
        QTest::qWait(50);
        QVERIFY(!view.grabWindow().isNull());
        view.rootObject()->setProperty("viewMode", 2);
        QTest::qWait(50);
        QVERIFY(!view.grabWindow().isNull());
        view.rootObject()->setProperty("viewMode", 0);
        QTest::qWait(50);
        auto folder = visualItem(view.rootObject(), "fileTile-Folder");
        QVERIFY(folder);
        QTest::mouseDClick(&view, Qt::LeftButton, Qt::NoModifier,
                           folder->mapToScene(QPointF(30, 30)).toPoint());
        QTRY_COMPARE(browser.path(), d.filePath("Folder"));
        for (int i = 0; i < 3; ++i) {
            auto button = visualItem(view.rootObject(), "traffic-" + QString::number(i));
            QVERIFY(button);
            QTest::mouseClick(&view, Qt::LeftButton, Qt::NoModifier,
                              button->mapToScene(QPointF(6, 6)).toPoint());
            QCOMPARE(ui.action, QStringList({"close", "minimize", "maximize"}).at(i));
        }
    }
};
QTEST_MAIN(FilesUI)
#include "files_ui_test.moc"
