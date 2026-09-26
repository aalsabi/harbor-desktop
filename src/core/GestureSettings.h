#pragma once
#include <QObject>
#include <QVariantMap>
#include <QDBusConnection>
#include <QTimer>
#include <memory>
class QLockFile;
class GestureSettings:public QObject {
 Q_OBJECT
 Q_CLASSINFO("D-Bus Interface","org.harbor.Gestures")
 Q_PROPERTY(QVariantMap mappings READ mappings NOTIFY changed)
 Q_PROPERTY(bool enabled READ enabled NOTIFY changed)
 Q_PROPERTY(bool active READ active NOTIFY changed)
 Q_PROPERTY(bool available READ available NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
public:
 explicit GestureSettings(QObject *parent=nullptr,QDBusConnection bus=QDBusConnection::sessionBus(),QString directory={});
 ~GestureSettings();
 QVariantMap mappings()const{return m_mappings;}bool enabled()const{return m_enabled;}bool active()const{return m_active;}bool available()const{return m_available;}bool busy()const{return m_busy;}QString error()const{return m_error;}
 Q_INVOKABLE void refresh();Q_INVOKABLE void restore();Q_INVOKABLE void apply(const QVariantMap &mappings,bool enabled);
 static QStringList keys();static bool validMappings(const QVariantMap &mappings);
 static QString script(const QVariantMap &mappings,const QString &service,const QString &path,const QString &token);
public slots:void Ready(const QString &token);
signals:void changed();
private:
 void readConfig();void load();void success();void fail(QString message);void unloadAfterFailure();
 QDBusConnection m_bus;QString m_directory,m_path,m_token,m_error,m_scriptPath;QVariantMap m_mappings,m_draft;QTimer m_timeout;
 std::unique_ptr<QLockFile> m_lock;
 int m_generation=0;
 bool m_enabled=false,m_targetEnabled=false,m_active=false,m_available=false,m_busy=false,m_autoRestore=false;
};
