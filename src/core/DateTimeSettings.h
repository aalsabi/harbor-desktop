#pragma once
#include <QObject>
#include <QDBusConnection>
#include <QVariantMap>
class QTimer;

class DateTimeSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentDateTime READ currentDateTime NOTIFY clockChanged)
    Q_PROPERTY(QString timezone READ timezone NOTIFY changed)
    Q_PROPERTY(QStringList timezones READ timezones CONSTANT)
    Q_PROPERTY(bool ntp READ ntp NOTIFY changed)
    Q_PROPERTY(bool canNtp READ canNtp NOTIFY changed)
    Q_PROPERTY(bool synchronized READ synchronized NOTIFY changed)
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
public:
    explicit DateTimeSettings(QObject* parent = nullptr);
    explicit DateTimeSettings(const QDBusConnection& bus, QObject* parent = nullptr);
    QString currentDateTime() const;
    QString timezone() const { return m_timezone; }
    QStringList timezones() const { return m_timezones; }
    bool ntp() const { return m_ntp; }
    bool canNtp() const { return m_canNtp; }
    bool synchronized() const { return m_synced; }
    bool available() const { return m_available; }
    bool busy() const { return m_busy; }
    QString error() const { return m_error; }
    Q_INVOKABLE void setActive(bool active);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setTimezone(const QString& zone);
    Q_INVOKABLE void setNtp(bool enabled);
    Q_INVOKABLE void setDateTime(const QString& date, const QString& time);
    static bool parseDateTime(const QString& date, const QString& time, const QString& zone,
                              qint64* microseconds);
signals:
    void changed();
    void clockChanged();
private slots:
    void propertiesChanged(const QString&, const QVariantMap&, const QStringList&);

private:
    void mutate(const QString& method, const QVariantList& args);
    void fail(const QString& message);
    QDBusConnection m_bus;
    QTimer* m_clock = nullptr;
    QStringList m_timezones;
    QString m_timezone, m_error;
    bool m_ntp = false, m_canNtp = false, m_synced = false, m_available = false, m_busy = false;
};
