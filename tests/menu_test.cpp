#include <QtTest>
#include <QDBusConnection>
#include <QDBusMetaType>
#include "core/GlobalMenu.h"
class MenuFixture : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.canonical.dbusmenu")
public:
    int clicked = -1;
public slots:
    uint GetLayout(int, int, QStringList, MenuLayout& tree) {
        MenuLayout item{11, {{"label", "_Open"}, {"enabled", true}}, {}};
        MenuLayout file{10,
                        {{"label", "_File"}, {"children-display", "submenu"}},
                        {QDBusVariant(QVariant::fromValue(item))}};
        tree = {0, {}, {QDBusVariant(QVariant::fromValue(file))}};
        return 1;
    }
    bool AboutToShow(int) { return false; }
    void Event(int id, QString event, QDBusVariant, uint) {
        if (event == "clicked")
            clicked = id;
    }
};
class MenuTest : public QObject {
    Q_OBJECT
private slots:
    void exportedMenuCanBeTraversedAndClicked() {
        GlobalMenu menu;
        MenuFixture fixture;
        auto bus = QDBusConnection::sessionBus();
        QVERIFY(bus.registerService("org.harbor.MenuTest"));
        QVERIFY(bus.registerObject("/Menu", &fixture, QDBusConnection::ExportAllSlots));
        menu.setSource("org.harbor.MenuTest", "/Menu");
        QTRY_COMPARE(menu.roots().size(), 1);
        QCOMPARE(menu.roots()[0].toMap()["label"].toString(), QString("File"));
        menu.select(10);
        QCOMPARE(menu.items().size(), 1);
        menu.trigger(11);
        QTRY_COMPARE(fixture.clicked, 11);
        bus.unregisterService("org.harbor.MenuTest");
        bus.unregisterObject("/Menu");
    }
};
QTEST_GUILESS_MAIN(MenuTest)
#include "menu_test.moc"
