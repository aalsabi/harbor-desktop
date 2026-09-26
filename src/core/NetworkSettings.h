#pragma once
#include <QObject>
#include <QVariantMap>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QTimer>
class NetworkSecretAgent;
#include <functional>
using NetworkProfile = QMap<QString, QVariantMap>;
using NetworkAddresses = QList<QVariantMap>;
Q_DECLARE_METATYPE(NetworkAddresses)
class NetworkSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap secretPrompt READ secretPrompt NOTIFY changed)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY changed)
    Q_PROPERTY(QVariantList networks READ networks NOTIFY changed)
    Q_PROPERTY(QVariantList profiles READ profiles NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(bool wirelessEnabled READ wirelessEnabled NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
public:
    explicit NetworkSettings(QObject* parent = nullptr, QDBusConnection bus = QDBusConnection::systemBus(),
                             QString importHelper = "/usr/libexec/harbor-network-import");
    ~NetworkSettings();
    QVariantMap secretPrompt() const { return m_secretPrompt; }
    Q_INVOKABLE void respondSecrets(bool accept, const QVariantMap& values);
    Q_INVOKABLE void cancelSecrets();
    QVariantList devices() const { return m_devices; }
    QVariantList networks() const { return m_networks; }
    QVariantList profiles() const { return m_profiles; }
    bool busy() const { return m_busy; }
    bool available() const { return m_available; }
    bool wirelessEnabled() const { return m_wireless; }
    QString error() const { return m_error; }
    Q_INVOKABLE void setActive(bool active);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setWirelessEnabled(bool enabled);
    Q_INVOKABLE void scan();
    Q_INVOKABLE void connectWifi(const QString& accessPoint, const QString& password);
    Q_INVOKABLE void activate(const QString& profile);
    Q_INVOKABLE void disconnectDevice(const QString& device);
    Q_INVOKABLE QVariantMap profileDraft(const QString& profile) const;
    Q_INVOKABLE void saveProfile(const QString& path, const QVariantMap& draft);
    Q_INVOKABLE void importVpn(const QString& file, const QString& type);
    static QString validateIp(const QVariantMap& draft, int family);
signals:
    void changed();
    void profileSaved(const QString& path);

private:
    void read(const QString& path, const QString& iface, const QString& method, const QVariantList& args,
              std::function<void(const QDBusMessage&)> done);
    void write(const QString& path, const QString& iface, const QString& method, const QVariantList& args,
               std::function<void(const QDBusMessage&)> done = {});
    friend class NetworkSecretAgent;
    NetworkSecretAgent* m_agent = nullptr;
    bool m_agentRegistered = false;
    QString m_activatingPath;
    QVariantMap m_secretPrompt;
    void activateWithAgent(const QString& path);
    void updatePreservingSecrets(const QString& path, NetworkProfile config, QStringList keys);
    void loadDevice(const QString& path);
    void loadProfile(const QString& path);
    void fail(const QString& message);
    bool knownDevice(const QString& path) const;
    QTimer m_refreshTimer, m_secretTimer;
    QDBusConnection m_bus;
    QString m_importHelper, m_error;
    QVariantList m_devices, m_networks, m_profiles;
    QMap<QString, NetworkProfile> m_configs;
    bool m_busy = false, m_available = false, m_wireless = false;
    int m_reads = 0;
};

class NetworkSecretAgent : public QObject, protected QDBusContext {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManager.SecretAgent")
public:
    explicit NetworkSecretAgent(NetworkSettings* owner) : QObject(owner), m_owner(owner) {}
    void respond(bool accept, const QVariantMap& values);
public slots:
    NetworkProfile GetSecrets(const NetworkProfile& connection, const QDBusObjectPath& path,
                              const QString& setting, const QStringList& hints, uint flags);
    void CancelGetSecrets(const QDBusObjectPath& path, const QString& setting);
    void SaveSecrets(const NetworkProfile&, const QDBusObjectPath&);
    void DeleteSecrets(const NetworkProfile&, const QDBusObjectPath&);

private:
    bool trusted() const;
    NetworkSettings* m_owner;
    QDBusMessage m_pending;
    bool m_waiting = false;
    QString m_setting;
    QStringList m_keys;
};
