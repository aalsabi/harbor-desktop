#pragma once
#include <QObject>
#include "NotificationPreferences.h"
#include <QVariantList>
#include <QVariantMap>
class Notifications:public QObject {
 Q_OBJECT
 Q_CLASSINFO("D-Bus Interface","org.freedesktop.Notifications")
 Q_PROPERTY(int attentionCount READ attentionCount NOTIFY changed)
 Q_PROPERTY(QVariantList items READ items NOTIFY changed)
 Q_PROPERTY(bool available READ available CONSTANT)
public:explicit Notifications(bool serve=true,QObject* parent=nullptr,QString preferencesPath={});int attentionCount()const{int count=0;for(auto row:rows)if(policy.attentionAllowed(row.toMap().value("policyId").toString()))++count;return count;}QVariantList items()const{return rows;}bool available()const{return registered;}
 Q_INVOKABLE void invoke(uint id,QString key);
public slots:
 uint Notify(QString app,uint replaces,QString icon,QString summary,QString body,QStringList actions,QVariantMap hints,int expire);
 void CloseNotification(uint id);
 QStringList GetCapabilities(){return {"body","actions"};}
 QString GetServerInformation(QString& vendor,QString& version,QString& spec){vendor="Harbor";version="0.1.0";spec="1.2";return "Harbor";}
signals:void changed();void NotificationClosed(uint id,uint reason);void ActionInvoked(uint id,QString key);
private:NotificationPreferences policy;void remove(uint id,uint reason);QVariantList rows;uint nextId=1;bool registered=false;QHash<uint,uint> generations;
};
