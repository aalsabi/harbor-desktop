#pragma once
#include <QObject>
#include <QVariantList>
#include "Command.h"
#include <QDBusConnection>
#include <functional>
class Printers : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList discovered READ discovered NOTIFY changed)
    Q_PROPERTY(QVariantMap selectedDevice READ selectedDevice NOTIFY changed)
    Q_PROPERTY(QVariantList drivers READ drivers NOTIFY changed)
    Q_PROPERTY(QVariantList options READ options NOTIFY changed)
    Q_PROPERTY(QString selectedPrinter READ selectedPrinter NOTIFY changed)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY changed)
    Q_PROPERTY(QString defaultPrinter READ defaultPrinter NOTIFY changed)
    Q_PROPERTY(QString message READ message NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool available READ available NOTIFY changed)
public:
    explicit Printers(QObject* parent = nullptr);
    explicit Printers(const QDBusConnection& connection, QObject* parent = nullptr);
    QVariantList drivers() const { return m_drivers; }
    QVariantList options() const { return m_options; }
    QString selectedPrinter() const { return m_selected; }
    QVariantList discovered() const { return m_discovered; }
    QVariantMap selectedDevice() const { return m_device; }
    Q_INVOKABLE void discover();
    Q_INVOKABLE void selectDevice(const QString& uri);
    static QVariantList parseDiscovery(const QString& output);
    static bool validDeviceUri(const QString& uri);
    Q_INVOKABLE void loadDrivers();
    Q_INVOKABLE void loadOptions(const QString& name);
    Q_INVOKABLE void setOption(const QString& name, const QString& key, const QString& value);
    Q_INVOKABLE void addPrinter(const QString& name, const QString& uri, const QString& driver);
    Q_INVOKABLE void removePrinter(const QString& name, bool confirmed = false);
    Q_INVOKABLE void cancelJob(const QString& id, bool confirmed = false);
    static QVariantList parseDrivers(const QString& output);
    static QVariantList parseOptions(const QString& output);
    static bool validUri(const QString& uri);
    QVariantList devices() const { return m_devices; }
    QString defaultPrinter() const { return m_default; }
    QString message() const { return m_message; }
    bool busy() const { return m_busy; }
    bool available() const { return m_available; }
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setDefault(const QString& name);
    static QVariantList parsePrinters(const QString& output);
    static QString parseDefault(const QString& output);
    static QVariantList parseJobs(const QString& output);
    static bool validName(const QString& name);
signals:
    void changed();

private:
    bool hasPrinter(const QString& name) const;
    void adminCall(const QString& method, const QVariantList& args, std::function<void()> after = {});
    QDBusConnection m_bus;
    QVariantList m_drivers, m_options, m_discovered;
    QVariantMap m_device;
    QString m_selected;
    void completed(bool ok, const QString& output);
    Command m_command;
    QVariantList m_devices;
    QString m_default, m_message, m_lpstat, m_lpoptions;
    bool m_busy = false, m_available = false;
    int m_stage = 0;
};
