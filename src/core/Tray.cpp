#include "Tray.h"
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QTimer>
Tray::Tray(bool serve, QObject* p) : QObject(p), watcher(this) {
    if (!serve)
        return;
    auto bus = QDBusConnection::sessionBus();
    serving = bus.registerService("org.kde.StatusNotifierWatcher");
    if (!serving)
        return;
    bus.registerObject("/StatusNotifierWatcher", this,
                       QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals |
                           QDBusConnection::ExportAllProperties);
    watcher.setConnection(bus);
    watcher.setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(&watcher, &QDBusServiceWatcher::serviceUnregistered, this, [this](QString service) {
        for (int i = rows.size() - 1; i >= 0; --i)
            if (rows[i].toMap()["service"] == service) {
                auto id = rows[i].toMap()["id"].toString();
                rows.removeAt(i);
                emit StatusNotifierItemUnregistered(id);
            }
        emit changed();
    });
    auto t = new QTimer(this);
    connect(t, &QTimer::timeout, this, [this] {
        for (auto id : registered())
            update(id);
    });
    t->start(5000);
}
QStringList Tray::registered() const {
    QStringList out;
    for (auto row : rows)
        out << row.toMap()["id"].toString();
    return out;
}
void Tray::RegisterStatusNotifierItem(QString s) {
    if (!serving)
        return;
    QString path = "/StatusNotifierItem";
    if (s.startsWith('/')) {
        path = s;
        s = calledFromDBus() ? message().service() : QString();
    }
    if (s.isEmpty() || path.isEmpty())
        return;
    QString id = s + path;
    if (registered().contains(id))
        return;
    rows.append(QVariantMap{
        {"id", id}, {"service", s}, {"path", path}, {"icon", "app"}, {"title", s}, {"status", "Active"}});
    watcher.addWatchedService(s);
    emit StatusNotifierItemRegistered(id);
    emit changed();
    update(id);
}
void Tray::RegisterStatusNotifierHost(QString) {
    emit StatusNotifierHostRegistered();
}
void Tray::update(QString id) {
    QVariantMap entry;
    for (auto row : rows)
        if (row.toMap()["id"] == id)
            entry = row.toMap();
    if (entry.isEmpty())
        return;
    auto msg = QDBusMessage::createMethodCall(entry["service"].toString(), entry["path"].toString(),
                                              "org.freedesktop.DBus.Properties", "GetAll");
    msg << QString("org.kde.StatusNotifierItem");
    auto w = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(msg, 1500), this);
    connect(w, &QDBusPendingCallWatcher::finished, this, [this, w, id] {
        QDBusPendingReply<QVariantMap> r = *w;
        w->deleteLater();
        if (r.isError())
            return;
        auto props = r.value();
        for (int i = 0; i < rows.size(); ++i)
            if (rows[i].toMap()["id"] == id) {
                auto m = rows[i].toMap();
                m["title"] = props.value("Title");
                m["icon"] = props.value("IconName");
                m["status"] = props.value("Status");
                rows[i] = m;
                emit changed();
                break;
            }
    });
}
void Tray::activate(QString id, bool context) {
    for (auto row : rows) {
        auto m = row.toMap();
        if (m["id"] != id)
            continue;
        auto msg = QDBusMessage::createMethodCall(m["service"].toString(), m["path"].toString(),
                                                  "org.kde.StatusNotifierItem",
                                                  context ? "ContextMenu" : "Activate");
        msg << 0 << 0;
        QDBusConnection::sessionBus().asyncCall(msg, 2000);
        return;
    }
}
