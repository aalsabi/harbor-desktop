#include "WindowMenu.h"
#include <QWindow>
#include <QGuiApplication>
#include <QDBusConnection>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/appmenu.h>
#include <KWayland/Client/surface.h>
using namespace KWayland::Client;
void attachWindowMenu(QWindow* window, const QString& objectPath) {
    if (QGuiApplication::platformName() != QStringLiteral("wayland"))
        return;
    auto connection = ConnectionThread::fromApplication(window);
    if (!connection)
        return;
    auto registry = new Registry(window);
    QObject::connect(registry, &Registry::appMenuAnnounced, window,
                     [window, registry, objectPath](quint32 name, quint32 version) {
                         auto surface = Surface::fromWindow(window);
                         if (!surface)
                             return;
                         auto manager = registry->createAppMenuManager(name, version, window);
                         auto menu = manager->create(surface, window);
                         menu->setAddress(QDBusConnection::sessionBus().baseService(), objectPath);
                     });
    registry->create(connection);
    registry->setup();
}
