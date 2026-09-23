#pragma once
#include <QObject>
#include <QVariantList>
#include <QHash>
#include <QTimer>
class Applications:public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantList entries READ entries NOTIFY changed)
 Q_PROPERTY(QStringList pendingLaunches READ pendingLaunches NOTIFY changed)
 Q_PROPERTY(QString message READ message NOTIFY changed)
public:explicit Applications(QObject* parent=nullptr);QVariantList entries()const{return list;}
 QStringList pendingLaunches()const{return pending.keys();}
 QString message()const{return status;}
 Q_INVOKABLE void setWindows(QVariantList windows);
 Q_INVOKABLE QString desktopForTool(QString tool)const;
 Q_INVOKABLE QString iconForAppId(QString appId) const;
 Q_INVOKABLE void refresh();Q_INVOKABLE bool launch(QString id);
signals:void changed();void error(QString message);void activateRequested(QString windowId);
private:
 void clearPending(QString id);
 QString desktopKey(QString appId)const;
 QVariantList list,runningWindows;QHash<QString,QString> appIcons,desktopAliases,executables;QHash<QString,QTimer*> pending;QString status;
};
