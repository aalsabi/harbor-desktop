#include "Controller.h"
#include "Palette.h"
#include "SettingsServices.h"
#include "ShellOptions.h"
#include "Surfaces.h"
#include "core/Accounts.h"
#include "core/Applications.h"
#include "core/Files.h"
#include "core/FilesMenu.h"
#include "core/GlobalMenu.h"
#include "core/Icons.h"
#include "core/Keyboard.h"
#include "core/Notifications.h"
#include "core/Preferences.h"
#include "core/Region.h"
#include "core/SystemServices.h"
#include "core/Translation.h"
#include "core/Tray.h"
#include "kwin/WindowMenu.h"
#include "kwin/WindowModel.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickView>
#include <QScreen>
#include <QTextStream>
#include <QTimer>

int main(int argc, char** argv) {
    QQuickStyle::setStyle("Basic");
    QQuickWindow::setDefaultAlphaBuffer(true);
    QGuiApplication app(argc, argv);
    Icons::configureTheme();
    app.setApplicationName("Harbor");
    app.setOrganizationName("Harbor");
    app.setFont(QFont("Noto Sans", 10));
    app.setDesktopFileName("org.harbor.Shell");
    app.setQuitOnLastWindowClosed(false);
    const auto options = ShellOptions::parse(app.arguments());

    // Services shared by every surface. Construction order is also the reverse destruction order.
    Files browser;
    if (options.files)
        app.setDesktopFileName("org.harbor.Files");
    if (!options.initialPath.isEmpty())
        browser.navigate(options.initialPath);
    Preferences preferences;
    Keyboard keyboard;
    Region region;
    SettingsServices settingsServices;
    applyPalette(preferences);
    QObject::connect(&preferences, &Preferences::changed, &app, [&] {
        applyPalette(preferences);
    });
    Applications applications;
    SystemServices services;
    WindowModel windows;
    Controller controller;
    Notifications notifications(options.session());
    Tray tray(options.session());
    Accounts accounts;
    QTimer::singleShot(0, &accounts, &Accounts::refresh);
    QObject::connect(&windows, &WindowModel::changed, &applications, [&] {
        applications.setWindows(windows.windows());
    });
    QObject::connect(&applications, &Applications::activateRequested, &windows, &WindowModel::activate);
    QObject::connect(&applications, &Applications::error, &notifications, [&](QString error) {
        notifications.Notify("Harbor", 0, "dialog-error", QObject::tr("Application launch"), error, {}, {},
                             10000);
    });
    GlobalMenu menu;
    QObject::connect(&menu, &GlobalMenu::activated, &controller, &Controller::dismiss);
    QObject::connect(&windows, &WindowModel::changed, &menu, [&] {
        if (!options.preview)
            menu.setSource(windows.menuService(), windows.menuPath());
    });

    // QML engine, translations and the objects QML reaches by name.
    Translation translation(preferences);
    app.setLayoutDirection(translation.rightToLeft() ? Qt::RightToLeft : Qt::LeftToRight);
    QQmlEngine engine;
    QObject::connect(&translation, &Translation::changed, &engine, [&] {
        app.setLayoutDirection(translation.rightToLeft() ? Qt::RightToLeft : Qt::LeftToRight);
        engine.retranslate();
    });
    engine.addImageProvider("icons", new Icons);
    auto ctx = engine.rootContext();
    settingsServices.expose(ctx);
    ctx->setContextProperty("Region", &region);
    ctx->setContextProperty("Keyboard", &keyboard);
    ctx->setContextProperty("HarborVersion", QStringLiteral(HARBOR_VERSION));
    ctx->setContextProperty("Browser", &browser);
    ctx->setContextProperty("Notifications", &notifications);
    ctx->setContextProperty("GlobalMenu", &menu);
    ctx->setContextProperty("Tray", &tray);
    ctx->setContextProperty("Accounts", &accounts);
    ctx->setContextProperty("Prefs", &preferences);
    ctx->setContextProperty("Apps", &applications);
    ctx->setContextProperty("System", &services);
    ctx->setContextProperty("Windows", &windows);
    ctx->setContextProperty("UI", &controller);

    // org.harbor.Shell D-Bus interface, served only by the session instance.
    controller.onList = [&] {
        return QString::fromUtf8(
            QJsonDocument(QJsonArray::fromVariantList(windows.windows())).toJson(QJsonDocument::Compact));
    };
    controller.onActivate = [&](QString id) {
        windows.activate(id);
    };
    controller.onWindowClose = [&](QString id) {
        windows.close(id);
    };
    if (options.session()) {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.registerService("org.harbor.Shell"))
            return 4;
        bus.registerObject("/Shell", &controller, QDBusConnection::ExportAllSlots);
        QTimer::singleShot(800, &settingsServices.accessibility, &AccessibilitySettings::restore);
        QTimer::singleShot(1200, &settingsServices.gestures, &GestureSettings::restore);
    }

    // Surfaces: panels per screen in the session, or a single window in the other modes.
    QList<QQuickView*> surfaces;
    QQuickView* popup = nullptr;
    auto make = [&](const QString& name, QScreen* screen, QSize size, SurfaceRole role) {
        return createSurface(engine, name, screen, size, role, options.preview);
    };
    controller.onWindowAction = [&](QString name) {
        QQuickView* v =
            popup ? popup
                  : ((options.settings || options.files) && !surfaces.isEmpty() ? surfaces[0] : nullptr);
        if (!v)
            return;
        if (name == "move")
            v->startSystemMove();
        else if (name == "resize")
            v->startSystemResize(Qt::RightEdge | Qt::BottomEdge);
        else if (name == "close")
            v->close();
        else if (name == "minimize")
            v->showMinimized();
        else if (name == "maximize") {
            if (v->visibility() == QWindow::Maximized)
                v->showNormal();
            else
                v->showMaximized();
        }
    };
    controller.onLogout = [&] {
        if (options.session())
            QTimer::singleShot(100, &app, &QCoreApplication::quit);
        else {
            auto message =
                QDBusMessage::createMethodCall("org.harbor.Shell", "/Shell", "org.harbor.Shell", "Logout");
            QDBusConnection::sessionBus().asyncCall(message, 2000);
        }
    };
    controller.onClose = [&] {
        if (popup) {
            popup->hide();
            popup->deleteLater();
            popup = nullptr;
        } else if (options.control)
            app.quit();
    };
    controller.onOpen = [&](QString page) {
        controller.dismiss();
        const auto spec = popupFor(page);
        popup = make(spec.name, app.primaryScreen(), spec.size, spec.role);
        if (popup) {
            if (spec.settings && page.startsWith("settings:"))
                popup->rootObject()->setProperty("section", page.mid(9));
            popup->requestActivate();
        }
    };
    if (!options.session()) {
        auto v = make(options.windowName(), app.primaryScreen(), options.windowSize(), SurfaceRole::Window);
        if (!v)
            return 2;
        surfaces << v;
        if (!options.settingsPage.isEmpty())
            v->rootObject()->setProperty("section", options.settingsPage);
        if (options.files || options.preview) {
            auto fileRoot =
                options.files ? v->rootObject() : v->rootObject()->findChild<QQuickItem*>("filesView");
            if (fileRoot) {
                auto exporter = new FilesMenu(v);
                fileRoot->setProperty("menuExporter", QVariant::fromValue(static_cast<QObject*>(exporter)));
                if (options.files)
                    attachWindowMenu(v, exporter->objectPath());
                else
                    menu.setSource(QDBusConnection::sessionBus().baseService(), exporter->objectPath());
            }
        }
        QObject::connect(v, &QWindow::visibleChanged, &app, [&app, v] {
            if (!v->isVisible())
                app.quit();
        });
        if (options.screenshot) {
            if (options.screenshotSize.isValid())
                v->resize(options.screenshotSize);
            QTimer::singleShot(1800, &app, [&app, v, path = options.screenshotPath] {
                bool ok = v->grabWindow().save(path);
                app.exit(ok ? 0 : 3);
            });
        }
    } else {
        auto addScreen = [&](QScreen* screen) {
            auto geo = screen->geometry();
            const QList<std::pair<QString, SurfaceRole>> panels{{"Desktop", SurfaceRole::Background},
                                                                {"MenuBar", SurfaceRole::Panel},
                                                                {"Dock", SurfaceRole::Dock}};
            for (const auto& [name, role] : panels) {
                QSize size(role == SurfaceRole::Dock ? 720 : geo.width(), role == SurfaceRole::Background
                                                                              ? geo.height()
                                                                          : role == SurfaceRole::Panel ? 38
                                                                                                       : 78);
                auto v = make(name, screen, size, role);
                if (v) {
                    surfaces << v;
                    QObject::connect(screen, &QScreen::geometryChanged, v, [v, role](QRect r) {
                        if (role != SurfaceRole::Dock)
                            v->setWidth(r.width());
                        if (role == SurfaceRole::Background)
                            v->setHeight(r.height());
                    });
                }
            }
        };
        for (auto screen : app.screens())
            addScreen(screen);
        QObject::connect(&app, &QGuiApplication::screenAdded, &app, addScreen);
        QObject::connect(&app, &QGuiApplication::screenRemoved, &app, [&](QScreen* screen) {
            for (int i = surfaces.size() - 1; i >= 0; --i)
                if (surfaces[i]->property("harborScreen").value<QScreen*>() == screen) {
                    delete surfaces.takeAt(i);
                }
        });
    }
    if (options.diagnose)
        QTimer::singleShot(1200, &app, [&] {
            QJsonObject j{
                {"windowManagement", windows.available()},
                {"platform", app.platformName()},
                {"screens", app.screens().size()},
                {"windows", windows.windows().size()},
                {"bluetoothAvailable", services.state().value("bluetoothAvailable").toBool()},
                {"bluetoothStatus", services.state().value("bluetoothStatus").toString()},
                {"bluetoothDeviceCount", services.state().value("bluetoothDevices").toList().size()}};
            QTextStream(stdout) << QJsonDocument(j).toJson();
            app.quit();
        });
    int result = app.exec();
    controller.dismiss();
    qDeleteAll(surfaces);
    return result;
}
