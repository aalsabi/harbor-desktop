#include "Surfaces.h"
#include <KWindowEffects>
#include <LayerShellQt/Window>
#include <QGuiApplication>
#include <QQuickView>
#include <QScreen>

QQuickView* createSurface(QQmlEngine& engine, const QString& name, QScreen* screen, QSize size,
                          SurfaceRole role, bool preview) {
    auto view = new QQuickView(&engine, nullptr);
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setColor(Qt::transparent);
    view->setScreen(screen);
    view->resize(size);
    if (name == "Settings" || name == "Files") {
        view->setFlags(Qt::FramelessWindowHint);
        view->setMinimumSize(QSize(740, 560));
    }
    if (name == "Files")
        view->setMinimumSize(QSize(1000, 600));
    // Keep the initial geometry on the target output: Qt may otherwise
    // select the primary output when creating the native Wayland surface.
    view->setPosition(screen->geometry().topLeft());
    view->setTitle("Harbor — " + name);
    view->setProperty("harborScreen", QVariant::fromValue(screen));
    const bool layered = !preview && QGuiApplication::platformName() == "wayland";
    if (role != SurfaceRole::Window && layered) {
        view->setFlags(Qt::FramelessWindowHint);
        auto layer = LayerShellQt::Window::get(view);
        layer->setScope("harbor-" + name.toLower());
        using W = LayerShellQt::Window;
        switch (role) {
        case SurfaceRole::Background:
            layer->setLayer(W::LayerBackground);
            layer->setAnchors(W::Anchors(W::AnchorTop) | W::AnchorBottom | W::AnchorLeft | W::AnchorRight);
            layer->setExclusiveZone(-1);
            break;
        case SurfaceRole::Panel:
            layer->setLayer(W::LayerTop);
            layer->setAnchors(W::Anchors(W::AnchorTop) | W::AnchorLeft | W::AnchorRight);
            layer->setExclusiveZone(size.height());
            break;
        case SurfaceRole::Dock:
            layer->setLayer(W::LayerTop);
            layer->setAnchors(W::AnchorBottom);
            layer->setMargins(QMargins(0, 0, 0, 10));
            layer->setExclusiveZone(size.height() + 10);
            break;
        case SurfaceRole::Popup:
            layer->setLayer(W::LayerOverlay);
            layer->setAnchors(
                W::Anchors(W::AnchorTop) |
                (name == "HarborMenu" || name == "RecentItems" ? W::AnchorLeft : W::AnchorRight));
            layer->setMargins(QMargins(18, 42, 18, 0));
            layer->setExclusiveZone(0);
            break;
        case SurfaceRole::Window:
            break;
        }
        layer->setKeyboardInteractivity(role == SurfaceRole::Popup ? W::KeyboardInteractivityOnDemand
                                                                   : W::KeyboardInteractivityNone);
    }
    view->setSource(QUrl("qrc:/qml/" + name + ".qml"));
    if (view->status() == QQuickView::Error) {
        delete view;
        return nullptr;
    }
    view->show();
    if (role != SurfaceRole::Window && role != SurfaceRole::Background && layered)
        KWindowEffects::enableBlurBehind(view, true);
    return view;
}

PopupSpec popupFor(const QString& page) {
    const bool settings = page == "settings" || page.startsWith("settings:");
    QString name = page == "harbor"          ? "HarborMenu"
                   : page == "bluetooth"     ? "BluetoothMenu"
                   : page == "recent"        ? "RecentItems"
                   : page == "input"         ? "InputMenu"
                   : page == "menu"          ? "AppMenu"
                   : page == "notifications" ? "NotificationCenter"
                   : page == "launcher"      ? "Launcher"
                   : settings                ? "Settings"
                   : page == "windows"       ? "WindowList"
                                             : "ControlCenter";
    int width = name == "Settings" ? 960 : name == "Launcher" ? 680 : name == "InputMenu" ? 320 : 390;
    int height = name == "Settings"     ? 680
                 : name == "Launcher"   ? 560
                 : name == "InputMenu"  ? 320
                 : name == "HarborMenu" ? 450
                                        : 560;
    return {name, QSize(width, height), name == "Settings" ? SurfaceRole::Window : SurfaceRole::Popup,
            settings};
}
