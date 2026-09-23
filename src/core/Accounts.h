#pragma once
#include <QObject>
#include <QVariantList>
class Accounts:public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantList users READ users NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
public:using QObject::QObject;QVariantList users()const{return rows;}QString error()const{return problem;}bool busy()const{return working;}
 Q_INVOKABLE void refresh();Q_INVOKABLE void setRealName(QString path,QString name);
signals:void changed();
private:QVariantList rows;QString problem;bool working=false;uint generation=0;
};
