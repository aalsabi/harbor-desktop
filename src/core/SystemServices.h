#pragma once
#include <QObject>
#include <QVariantMap>
#include <QQueue>
#include <QHash>
#include "Command.h"
#include "Bluetooth.h"
class SystemServices:public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantMap state READ state NOTIFY changed)
 Q_PROPERTY(QString message READ message NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
public:explicit SystemServices(QObject* parent=nullptr,bool poll=true);QVariantMap state()const{return values;}QString message()const{return status;}bool busy()const{return active>0;}
 Q_INVOKABLE void refreshRecent();
 Q_INVOKABLE void openRecent(QString url);
 Q_INVOKABLE void applyDisplay(int output,double scale,QString mode);
 Q_INVOKABLE void confirmDisplay();
 Q_INVOKABLE void refresh();Q_INVOKABLE void action(QString name,QVariant value={});Q_INVOKABLE void openTool(QString name);
signals:void changed();
private:Bluetooth* bluetooth=nullptr;struct Pending{QString program;QStringList args;QString key;};QQueue<Pending> pending;QHash<QString,uint> generations;void querySystem();void startNext();void query(QString key,QString program,QStringList args);void execute(QString program,QStringList args,QString key={});QVariantMap values;QString status;int active=0;QString displayTransaction;QTimer* displayPoll=nullptr;
};
