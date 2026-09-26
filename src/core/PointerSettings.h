#pragma once
#include <QObject>
#include <QVariantList>
#include <QDBusConnection>
#include <functional>
#include <QTimer>

class PointerSettings : public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
 Q_PROPERTY(bool available READ available NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
public:
 explicit PointerSettings(QObject *parent=nullptr, QDBusConnection bus=QDBusConnection::sessionBus());
 QVariantList devices() const { return m_devices; }
 bool available() const { return m_available; }
 bool busy() const { return m_busy; }
 QString error() const { return m_error.isEmpty()?m_refreshError:m_error; }
 Q_INVOKABLE void setActive(bool active);
 Q_INVOKABLE void setSetting(const QString &sysName, const QString &property, const QVariant &value);
public slots:
 void refresh();
signals:
 void changed();
 void devicesChanged();
private:
 void readDevice(const QString &, std::function<void(QVariantMap, QString)>);
 void finish();
 void updateDevices(const QVariantList &devices);
 QDBusConnection m_bus;
 QTimer m_timer;
 QVariantList m_devices;
 QString m_error, m_refreshError;
 bool m_available=false, m_busy=false, m_refreshQueued=false;
};
