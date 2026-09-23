#pragma once
#include <QObject>
#include <QVariantMap>
class Keyboard:public QObject{
 Q_OBJECT
 Q_PROPERTY(QVariantMap state READ state NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
 Q_PROPERTY(QString message READ message NOTIFY changed)
 Q_PROPERTY(QString activeLabel READ activeLabel NOTIFY activeChanged)
 Q_PROPERTY(QString activeName READ activeName NOTIFY activeChanged)
 Q_PROPERTY(bool available READ available NOTIFY activeChanged)
public:
 explicit Keyboard(QObject* parent=nullptr);
 QVariantMap state()const{return values;}bool busy()const{return working;}QString message()const{return status;}
 QString activeLabel()const{return label;}
 QString activeName()const{return layoutName;}
 bool available()const{return liveAvailable;}
 Q_INVOKABLE void refresh();
 Q_INVOKABLE void apply(QStringList layouts,QString shortcut);
 Q_INVOKABLE void switchNext();
public slots:
 void refreshActive();
signals:void changed();void activeChanged();
private:
 QString label=QString::fromUtf8("⌨"),layoutName;bool liveAvailable=false;uint liveGeneration=0;
 void clearActive();
 void run(QStringList args,bool applying);QVariantMap values;bool working=false;QString status;
};
