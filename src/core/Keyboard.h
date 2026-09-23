#pragma once
#include <QObject>
#include <QVariantMap>
class Keyboard:public QObject{
 Q_OBJECT
 Q_PROPERTY(QVariantMap state READ state NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
 Q_PROPERTY(QString message READ message NOTIFY changed)
public:
 explicit Keyboard(QObject* parent=nullptr);
 QVariantMap state()const{return values;}bool busy()const{return working;}QString message()const{return status;}
 Q_INVOKABLE void refresh();
 Q_INVOKABLE void apply(QStringList layouts,QString shortcut);
 Q_INVOKABLE void switchNext();
signals:void changed();
private:void run(QStringList args,bool applying);QVariantMap values;bool working=false;QString status;
};
