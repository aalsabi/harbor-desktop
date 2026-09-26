#pragma once
#include <QObject>
#include <QDBusContext>
#include <QVariantList>
#include <QDBusServiceWatcher>
class Tray : public QObject, protected QDBusContext {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.StatusNotifierWatcher")
    Q_PROPERTY(QStringList RegisteredStatusNotifierItems READ registered)
    Q_PROPERTY(bool IsStatusNotifierHostRegistered READ available)
    Q_PROPERTY(int ProtocolVersion READ protocolVersion)
    Q_PROPERTY(QVariantList items READ items NOTIFY changed)
public:
    explicit Tray(bool serve = true, QObject* parent = nullptr);
    QStringList registered() const;
    bool available() const { return serving; }
    int protocolVersion() const { return 0; }
    QVariantList items() const { return rows; }
    Q_INVOKABLE void activate(QString id, bool context = false);
public slots:
    void RegisterStatusNotifierItem(QString service);
    void RegisterStatusNotifierHost(QString service);
signals:
    void StatusNotifierItemRegistered(QString service);
    void StatusNotifierItemUnregistered(QString service);
    void StatusNotifierHostRegistered();
    void changed();

private:
    void update(QString id);
    QVariantList rows;
    bool serving = false;
    QDBusServiceWatcher watcher;
};
