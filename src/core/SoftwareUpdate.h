#pragma once
#include <QObject>
#include <QVariantList>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QTimer>
class SoftwareUpdate : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList packages READ packages NOTIFY changed)
    Q_PROPERTY(QVariantList preview READ preview NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool readyToInstall READ readyToInstall NOTIFY changed)
    Q_PROPERTY(bool checked READ checked NOTIFY changed)
    Q_PROPERTY(int percentage READ percentage NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QString restart READ restart NOTIFY changed)
public:
    explicit SoftwareUpdate(QObject* parent = nullptr);
    explicit SoftwareUpdate(const QDBusConnection& connection, QObject* parent = nullptr);
    QVariantList packages() const { return m_packages; }
    QVariantList preview() const { return m_preview; }
    bool busy() const { return m_busy; }
    bool readyToInstall() const { return m_ready; }
    bool checked() const { return m_checked; }
    int percentage() const { return m_percentage; }
    QString status() const { return m_status; }
    QString error() const { return m_error; }
    QString restart() const { return m_restart; }
    Q_INVOKABLE void checkUpdates(bool refreshCache = false);
    Q_INVOKABLE void prepare(const QStringList& ids);
    Q_INVOKABLE void install(bool confirmed = false);
    Q_INVOKABLE void discardPreview();
    static constexpr qulonglong OnlyTrusted = qulonglong(1) << 1;
    static constexpr qulonglong Simulate = qulonglong(1) << 2;
signals:
    void changed();
private slots:
    void transactionSignal(const QDBusMessage& message);
    void propertiesSignal(const QDBusMessage& message);

private:
    enum Mode { List, Refresh, Preview, Install };
    void start(Mode mode);
    void invoke(const QString& path, const QString& method, const QVariantList& args);
    void fail(const QString& error);
    void disconnectTransaction();
    void finish(uint exit);
    QVariantMap package(uint info, const QString& id, const QString& summary) const;
    QDBusConnection bus;
    QVariantList m_packages, m_preview;
    QStringList m_selected;
    QString m_path, m_status, m_error, m_restart;
    bool m_busy = false, m_ready = false, m_checked = false;
    int m_percentage = -1;
    Mode m_mode = List;
    QTimer m_watchdog;
};
