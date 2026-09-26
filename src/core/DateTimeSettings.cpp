#include "DateTimeSettings.h"
#include <QDateTime>
#include <QTimeZone>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QTimer>
namespace {
const QString service = QStringLiteral("org.freedesktop.timedate1");
const QString path = QStringLiteral("/org/freedesktop/timedate1");
} // namespace
DateTimeSettings::DateTimeSettings(QObject* parent)
    : DateTimeSettings(QDBusConnection::systemBus(), parent) {}
DateTimeSettings::DateTimeSettings(const QDBusConnection& bus, QObject* parent)
    : QObject(parent), m_bus(bus) {
    for (const auto& zone : QTimeZone::availableTimeZoneIds())
        m_timezones.append(QString::fromUtf8(zone));
    m_clock = new QTimer(this);
    m_clock->setInterval(1000);
    connect(m_clock, &QTimer::timeout, this, &DateTimeSettings::clockChanged);
    m_bus.connect(service, path, "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
                  SLOT(propertiesChanged(QString, QVariantMap, QStringList)));
}
QString DateTimeSettings::currentDateTime() const {
    if (!m_available || m_timezone.isEmpty())
        return {};
    const QTimeZone zone(m_timezone.toUtf8());
    return zone.isValid() ? QDateTime::currentDateTimeUtc().toTimeZone(zone).toString("yyyy-MM-dd  HH:mm:ss")
                          : QString();
}
void DateTimeSettings::propertiesChanged(const QString& iface, const QVariantMap&, const QStringList&) {
    if (iface == service && !m_busy)
        refresh();
}
void DateTimeSettings::fail(const QString& message) {
    m_error = message;
    emit changed();
}
void DateTimeSettings::setActive(bool active) {
    if (active) {
        m_clock->start();
        refresh();
    } else
        m_clock->stop();
}
void DateTimeSettings::refresh() {
    if (m_busy)
        return;
    m_busy = true;
    emit changed();
    auto msg = QDBusMessage::createMethodCall(service, path, "org.freedesktop.DBus.Properties", "GetAll");
    msg << service;
    auto* watcher = new QDBusPendingCallWatcher(m_bus.asyncCall(msg), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
        QDBusPendingReply<QVariantMap> reply = *watcher;
        watcher->deleteLater();
        m_busy = false;
        if (reply.isError()) {
            m_available = false;
            m_error = reply.error().message();
        } else {
            const auto p = reply.value();
            m_timezone = p.value("Timezone").toString();
            m_ntp = p.value("NTP").toBool();
            m_canNtp = p.value("CanNTP").toBool();
            m_synced = p.value("NTPSynchronized").toBool();
            m_available = true;
            m_error.clear();
        }
        emit changed();
        emit clockChanged();
    });
}
void DateTimeSettings::mutate(const QString& method, const QVariantList& args) {
    if (m_busy || !m_available)
        return;
    m_busy = true;
    m_error.clear();
    emit changed();
    auto msg = QDBusMessage::createMethodCall(service, path, service, method);
    msg.setArguments(args);
    auto* watcher = new QDBusPendingCallWatcher(m_bus.asyncCall(msg, 120000), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
        QDBusPendingReply<> reply = *watcher;
        watcher->deleteLater();
        m_busy = false;
        if (reply.isError()) {
            m_error = reply.error().message();
            emit changed();
        } else
            refresh();
    });
}
void DateTimeSettings::setTimezone(const QString& zone) {
    if (!m_timezones.contains(zone)) {
        fail(tr("Choose a valid time zone."));
        return;
    }
    mutate("SetTimezone", {zone, true});
}
void DateTimeSettings::setNtp(bool enabled) {
    if (!m_canNtp) {
        fail(tr("Automatic time is unavailable on this system."));
        return;
    }
    mutate("SetNTP", {enabled, true});
}
bool DateTimeSettings::parseDateTime(const QString& date, const QString& time, const QString& zone,
                                     qint64* microseconds) {
    const auto d = QDate::fromString(date, "yyyy-MM-dd");
    const auto t = QTime::fromString(time, "HH:mm:ss");
    const QTimeZone tz(zone.toUtf8());
    if (!d.isValid() || d.toString("yyyy-MM-dd") != date || !t.isValid() || t.toString("HH:mm:ss") != time ||
        !tz.isValid())
        return false;
    const QDateTime dt(d, t, tz);
    if (!dt.isValid() || dt.date() != d || dt.time() != t || d.year() < 1970 || d.year() > 9999)
        return false;
    if (microseconds)
        *microseconds = dt.toMSecsSinceEpoch() * 1000;
    return true;
}
void DateTimeSettings::setDateTime(const QString& date, const QString& time) {
    if (m_ntp) {
        fail(tr("Turn off automatic time before setting the clock."));
        return;
    }
    qint64 usec = 0;
    if (!parseDateTime(date, time, m_timezone, &usec)) {
        fail(tr("Enter a valid date (YYYY-MM-DD) and time (HH:MM:SS)."));
        return;
    }
    mutate("SetTime", {QVariant::fromValue(usec), false, true});
}
