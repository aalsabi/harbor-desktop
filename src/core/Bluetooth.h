#pragma once
#include <QObject>
#include <QVariantMap>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QTimer>
#include <QDBusContext>
#include <QDBusMessage>
class BluetoothAgent;
using BluezInterfaces=QMap<QString,QVariantMap>;
using BluezObjects=QMap<QDBusObjectPath,BluezInterfaces>;
Q_DECLARE_METATYPE(BluezInterfaces)
Q_DECLARE_METATYPE(BluezObjects)
class Bluetooth:public QObject {
 Q_OBJECT
public:
 explicit Bluetooth(QObject* parent=nullptr,QDBusConnection bus=QDBusConnection::systemBus());
 ~Bluetooth();
 QVariantMap state()const{return values;}
 void action(QString name,QVariant value={});
 static QVariantMap summarize(const BluezObjects& objects);
signals:void changed();
public slots:void refresh();
private:
 friend class BluetoothAgent;
 void prompt(const QString &kind,const QString &device,const QString &code={});
 void clearPrompt();
 BluetoothAgent *agent=nullptr;
 QString pairingPath; bool agentRegistered=false;
 void pairDevice(const QString &path);
 void call(QString path,QString interface,QString method,QVariantList arguments={},bool discovery=false);
 QDBusConnection bus;QVariantMap values;BluezObjects objects;QString adapter,scanAdapter;QTimer scanTimer;int generation=0;bool pending=false,scanWanted=false;
};

class BluetoothAgent : public QObject, protected QDBusContext {
 Q_OBJECT
 Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent1")
public:
 explicit BluetoothAgent(Bluetooth *owner);
 void respond(bool accept,const QString &value={});
 void dismiss();
public slots:
 void Release();
 QString RequestPinCode(const QDBusObjectPath &device);
 uint RequestPasskey(const QDBusObjectPath &device);
 void DisplayPinCode(const QDBusObjectPath &device,const QString &code);
 void DisplayPasskey(const QDBusObjectPath &device,uint code,ushort entered);
 void RequestConfirmation(const QDBusObjectPath &device,uint code);
 void RequestAuthorization(const QDBusObjectPath &device);
 void AuthorizeService(const QDBusObjectPath &device,const QString &uuid);
 void Cancel();
private:
 bool trustedCaller() const;
 bool request(const QString &kind,const QDBusObjectPath &device,const QString &code={});
 Bluetooth *owner; QDBusMessage waiting; QString requestKind; bool hasWaiting=false;
};
