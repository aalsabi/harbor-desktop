#include "PointerSettings.h"
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDBusVariant>
#include <QRegularExpression>
#include <QTimer>
#include <cmath>
#include <memory>

namespace {
const QString service = QStringLiteral("org.kde.KWin");
const QString root = QStringLiteral("/org/kde/KWin/InputDevice");
const QString deviceInterface = QStringLiteral("org.kde.KWin.InputDevice");
QDBusMessage request(const QString& path, const QString& method, const QVariantList& args) {
    auto message = QDBusMessage::createMethodCall(service, path, "org.freedesktop.DBus.Properties", method);
    message.setArguments(args);
    return message;
}
bool supported(const QVariantMap& device, const QString& property) {
    if (!device.contains(property))
        return false;
    if (property == "tapToClick" || property == "tapAndDrag")
        return device.value("touchpad").toBool() && device.value("tapFingerCount").toInt() > 0;
    if (property == "disableWhileTyping" && !device.value("touchpad").toBool())
        return false;
    static const QMap<QString, QString> caps{{"naturalScroll", "supportsNaturalScroll"},
                                             {"leftHanded", "supportsLeftHanded"},
                                             {"pointerAcceleration", "supportsPointerAcceleration"},
                                             {"disableWhileTyping", "supportsDisableWhileTyping"}};
    return caps.contains(property) && device.value(caps.value(property)).toBool();
}
} // namespace
PointerSettings::PointerSettings(QObject* parent, QDBusConnection bus) : QObject(parent), m_bus(bus) {
    m_bus.connect(service, root, "org.kde.KWin.InputDeviceManager", "deviceAdded", this, SLOT(refresh()));
    m_bus.connect(service, root, "org.kde.KWin.InputDeviceManager", "deviceRemoved", this, SLOT(refresh()));
    auto watcher = new QDBusServiceWatcher(service, m_bus, QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &PointerSettings::refresh);
    // KWin exports device properties without change signals; periodically reread external changes.
    m_timer.setInterval(4000);
    connect(&m_timer, &QTimer::timeout, this, &PointerSettings::refresh);
}
void PointerSettings::setActive(bool active) {
    if (active) {
        m_timer.start();
        refresh();
    } else
        m_timer.stop();
}
void PointerSettings::updateDevices(const QVariantList& devices) {
    if (m_devices != devices) {
        m_devices = devices;
        emit devicesChanged();
    }
}
void PointerSettings::finish() {
    m_busy = false;
    emit changed();
    if (m_refreshQueued) {
        m_refreshQueued = false;
        QTimer::singleShot(0, this, &PointerSettings::refresh);
    }
}
void PointerSettings::readDevice(const QString& name, std::function<void(QVariantMap, QString)> done) {
    auto watcher = new QDBusPendingCallWatcher(
        m_bus.asyncCall(request(root + "/" + name, "GetAll", {deviceInterface}), 3000), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [watcher, done = std::move(done)] {
        QDBusPendingReply<QVariantMap> reply = *watcher;
        watcher->deleteLater();
        done(reply.isError() ? QVariantMap{} : reply.value(),
             reply.isError() ? reply.error().message() : QString{});
    });
}
void PointerSettings::refresh() {
    if (m_busy) {
        m_refreshQueued = true;
        return;
    }
    m_busy = true;
    emit changed();
    auto watcher = new QDBusPendingCallWatcher(
        m_bus.asyncCall(
            request(root, "Get", {QString("org.kde.KWin.InputDeviceManager"), QString("devicesSysNames")}),
            3000),
        this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
        QDBusPendingReply<QDBusVariant> reply = *watcher;
        watcher->deleteLater();
        if (reply.isError()) {
            m_available = false;
            updateDevices({});
            m_refreshError = reply.error().message();
            finish();
            return;
        }
        m_refreshError.clear();
        m_available = true;
        QStringList names = qdbus_cast<QStringList>(reply.value().variant());
        names.removeDuplicates();
        names.sort();
        names.removeIf([](const QString& name) {
            return !QRegularExpression("^[A-Za-z0-9_]+$").match(name).hasMatch();
        });
        if (names.isEmpty()) {
            updateDevices({});
            finish();
            return;
        }
        auto remaining = std::make_shared<int>(names.size());
        auto result = std::make_shared<QMap<QString, QVariantMap>>();
        for (const auto& name : names)
            readDevice(name, [this, name, remaining, result](QVariantMap device, QString error) {
                if (!error.isEmpty())
                    m_refreshError = error;
                if (error.isEmpty() && device.value("pointer").toBool() &&
                    !device.value("tabletTool").toBool() && !device.value("tabletPad").toBool() &&
                    !device.value("touch").toBool()) {
                    device["sysName"] = name;
                    for (const auto& property : {"naturalScroll", "leftHanded", "pointerAcceleration",
                                                 "tapToClick", "tapAndDrag", "disableWhileTyping"})
                        device[QString("can_") + property] = supported(device, property);
                    result->insert(name, device);
                }
                if (--*remaining == 0) {
                    QVariantList devices;
                    for (const auto& d : *result)
                        devices.append(d);
                    updateDevices(devices);
                    finish();
                }
            });
    });
}
void PointerSettings::setSetting(const QString& name, const QString& property, const QVariant& value) {
    if (m_busy)
        return;
    QVariantMap device;
    for (const auto& item : m_devices)
        if (item.toMap().value("sysName").toString() == name)
            device = item.toMap();
    if (!supported(device, property)) {
        m_error = tr("This setting is not supported by this device.");
        emit changed();
        return;
    }
    QVariant normalized;
    if (property == "pointerAcceleration") {
        bool ok = false;
        double speed = value.toDouble(&ok);
        if (!ok || !std::isfinite(speed) || speed < -1 || speed > 1) {
            m_error = tr("Tracking speed must be between -1 and 1.");
            emit changed();
            return;
        }
        normalized = speed;
    } else {
        if (value.metaType().id() != QMetaType::Bool) {
            m_error = tr("Expected an on or off value.");
            emit changed();
            return;
        }
        normalized = value.toBool();
    }
    m_busy = true;
    m_error.clear();
    emit changed();
    // KWin setters persist successful changes through their own libinput KConfig groups.
    // Never expose enabled or disableEventsOnExternalMouse: the sole pointer stays usable.
    auto watcher = new QDBusPendingCallWatcher(
        m_bus.asyncCall(request(root + "/" + name, "Set",
                                {deviceInterface, property, QVariant::fromValue(QDBusVariant(normalized))}),
                        3000),
        this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, name, property, normalized] {
        QDBusPendingReply<> reply = *watcher;
        watcher->deleteLater();
        if (reply.isError()) {
            m_error = reply.error().message();
            emit devicesChanged();
            m_refreshQueued = true;
            finish();
            return;
        }
        readDevice(name, [this, property, normalized](QVariantMap actual, QString error) {
            bool matches = actual.contains(property) &&
                           (property == "pointerAcceleration"
                                ? std::abs(actual.value(property).toDouble() - normalized.toDouble()) < 0.001
                                : actual.value(property) == normalized);
            if (!error.isEmpty())
                m_error = error;
            else if (!matches)
                m_error = tr("The device did not accept this change.");
            emit devicesChanged();
            m_refreshQueued = true;
            finish();
        });
    });
}
