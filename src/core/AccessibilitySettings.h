#pragma once
#include <QObject>
#include <QVariantMap>
#include <QDBusConnection>
#include <QSettings>
#include <QFileSystemWatcher>
#include <functional>
class AccessibilitySettings:public QObject{
 Q_OBJECT
 Q_PROPERTY(QVariantMap state READ state NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
public:explicit AccessibilitySettings(QObject*parent=nullptr,QString path={},QDBusConnection bus=QDBusConnection::sessionBus());QVariantMap state()const{return values;}bool busy()const{return working||restoring;}QString error()const{return problem;}
 Q_INVOKABLE void refresh();Q_INVOKABLE void setEffect(QString name,bool enabled);Q_INVOKABLE void zoom(QString action);Q_INVOKABLE void setScreenReader(bool enabled);void restore();
signals:void changed();
private:QDBusConnection bus;QSettings settings;QFileSystemWatcher watcher;QVariantMap values;QString problem;bool working=false;void loadReaderStatus();
 void queryEffects(std::function<void(bool)> done);
 void applyEffect(QString name,bool enabled);
 void verifyEffect(QString name,bool enabled,int attempts);
 void finish(QString error={});void restoreNext();QStringList restoreQueue,restoreErrors;bool restoring=false;
};
