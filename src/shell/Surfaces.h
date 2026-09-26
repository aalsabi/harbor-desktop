#pragma once
#include <QSize>
#include <QString>
class QQmlEngine;
class QQuickView;
class QScreen;

// How a surface is placed. Every role except Window becomes a layer-shell surface in the Wayland session.
enum class SurfaceRole { Window = -1, Background = 0, Panel = 1, Dock = 2, Popup = 3 };

// Creates and shows qrc:/qml/<name>.qml on `screen`, or returns nullptr if the QML fails to load.
// `preview` keeps every surface an ordinary window so the shell can run nested.
QQuickView* createSurface(QQmlEngine& engine, const QString& name, QScreen* screen, QSize size,
                          SurfaceRole role, bool preview);

// The popup opened by UI.open(page): its QML name, size and role.
struct PopupSpec {
    QString name;
    QSize size;
    SurfaceRole role;
    bool settings; // "settings" or "settings:<page>"
};
PopupSpec popupFor(const QString& page);
