#include <QtTest>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QQmlEngine>
#include <QQmlComponent>
#include "core/Icons.h"
class RecordingIcons : public Icons {
public:
    QColor color;
    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requested) override {
        auto pixmap = Icons::requestPixmap(id, size, requested);
        color = pixmap.toImage().pixelColor(pixmap.width() / 2, pixmap.height() / 2);
        return pixmap;
    }
};
class IconsTest : public QObject {
    Q_OBJECT
    QTemporaryDir root;
private slots:
    void initTestCase() {
        QDir().mkpath(root.path() + "/fixture/64x64/apps");
        QFile index(root.path() + "/fixture/index.theme");
        QVERIFY(index.open(QIODevice::WriteOnly));
        index.write(R"([Icon Theme]
Name=Fixture
Directories=64x64/apps
[64x64/apps]
Size=64
Type=Fixed
Context=Applications
)");
        index.close();
        QImage image(64, 64, QImage::Format_ARGB32);
        image.fill(Qt::red);
        QVERIFY(image.save(root.path() + "/fixture/64x64/apps/system-file-manager.png"));
        QVERIFY(image.save(root.path() + "/official #icon.png"));
        QIcon::setThemeSearchPaths({root.path()});
        QIcon::setThemeName("fixture");
    }
    void systemThemeWinsOverBundledArtwork() {
        Icons icons;
        QSize size;
        auto pix = icons.requestPixmap("system-file-manager", &size, QSize(64, 64));
        QCOMPARE(pix.toImage().pixelColor(32, 32), QColor(Qt::red));
    }
    void absoluteIconPathLoads() {
        Icons icons;
        QSize size;
        auto pix = icons.requestPixmap(root.path() + "/official #icon.png", &size, QSize(64, 64));
        QCOMPARE(pix.toImage().pixelColor(32, 32), QColor(Qt::red));
    }
    void encodedAbsoluteIconLoadsInQml() {
        QQmlEngine engine;
        auto provider = new RecordingIcons;
        engine.addImageProvider("icons", provider);
        QQmlComponent component(&engine);
        QByteArray qml = "import QtQuick; Image { width:64; height:64; source: \"image://icons/" +
                         QUrl::toPercentEncoding(root.path() + "/official #icon.png") + "\" }";
        component.setData(qml, QUrl());
        QScopedPointer<QObject> image(component.create());
        QVERIFY2(image, qPrintable(component.errorString()));
        QTRY_COMPARE(image->property("status").toInt(), 1);
        QCOMPARE(provider->color, QColor(Qt::red));
    }
    void kdeConfiguredThemeIsRead() {
        qputenv("XDG_CONFIG_HOME", root.path().toUtf8());
        QFile config(root.path() + "/kdeglobals");
        QVERIFY(config.open(QIODevice::WriteOnly));
        config.write("[Icons]\nTheme=fixture\n");
        config.close();
        QIcon::setThemeName("unrelated");
        Icons::configureTheme();
        QCOMPARE(QIcon::themeName(), QString("fixture"));
    }
    void harborThemeOverridesKde() {
        QDir().mkpath(root.path() + "/harbor");
        QFile preference(root.path() + "/harbor/settings.ini");
        QVERIFY(preference.open(QIODevice::WriteOnly));
        preference.write("[General]\niconTheme=fixture\n");
        preference.close();
        QFile kde(root.path() + "/kdeglobals");
        QVERIFY(kde.open(QIODevice::WriteOnly));
        kde.write("[Icons]\nTheme=other\n");
        kde.close();
        Icons::configureTheme();
        QCOMPARE(QIcon::themeName(), QString("fixture"));
    }
    void themeSearchIncludesInstalledDataDirectories() {
        qputenv("XDG_DATA_HOME", root.path().toUtf8());
        QDir().mkpath(root.path() + "/icons");
        QVERIFY(QFile::link(root.path() + "/fixture", root.path() + "/icons/fixture"));
        QIcon::setThemeSearchPaths({":/icons"});
        Icons::configureTheme();
        QVERIFY(QIcon::themeSearchPaths().contains(root.path() + "/icons"));
        QVERIFY(QIcon::hasThemeIcon("system-file-manager"));
    }
    void unknownIconsHaveFallback() {
        Icons icons;
        QSize size;
        QVERIFY(!icons.requestPixmap("harbor-missing-fixture-icon", &size, QSize(32, 32)).isNull());
    }
};
QTEST_MAIN(IconsTest)
#include "icons_test.moc"
