#pragma once
#include <QObject>
#include <QVariantList>
#include <QTimer>
#include "Command.h"
class PowerSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList batteries READ batteries NOTIFY batteriesChanged)
    Q_PROPERTY(int displayMinutes READ displayMinutes NOTIFY idlePreferencesChanged)
    Q_PROPERTY(int suspendMinutes READ suspendMinutes NOTIFY idlePreferencesChanged)
    Q_PROPERTY(bool idleAvailable READ idleAvailable NOTIFY changed)
    Q_PROPERTY(QString idleStatus READ idleStatus NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
public:
    explicit PowerSettings(QObject* parent = nullptr, QString configPath = {}, QString sysfsRoot = {});
    QVariantList batteries() const { return m_batteries; }
    int displayMinutes() const { return m_display; }
    int suspendMinutes() const { return m_suspend; }
    bool idleAvailable() const { return m_idleAvailable; }
    QString idleStatus() const { return m_idleStatus; }
    QString error() const { return m_error; }
    bool busy() const { return m_busy; }
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setActive(bool active);
    Q_INVOKABLE void saveIdle(int displayMinutes, int suspendMinutes);
    Q_INVOKABLE void setChargeLimits(QString battery, int start, int end);
    static QVariantList readBatteries(const QString& root);
signals:
    void changed();
    void batteriesChanged();
    void idlePreferencesChanged();

private:
    Command m_command;
    QTimer m_timer;
    QString m_configPath, m_sysfsRoot, m_error, m_idleStatus;
    QVariantList m_batteries;
    int m_display = 0, m_suspend = 0;
    bool m_idleAvailable = false, m_busy = false;
};
