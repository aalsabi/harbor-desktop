#pragma once
#include <QObject>
#include <QVariantList>
#include <QDBusArgument>
#include <QDBusVariant>
struct MenuLayout {int id=0;QVariantMap properties;QList<QDBusVariant> children;};
Q_DECLARE_METATYPE(MenuLayout)
QDBusArgument& operator<<(QDBusArgument&,const MenuLayout&);
const QDBusArgument& operator>>(const QDBusArgument&,MenuLayout&);
class GlobalMenu:public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantList roots READ roots NOTIFY changed)
 Q_PROPERTY(QVariantList items READ items NOTIFY changed)
public:explicit GlobalMenu(QObject* p=nullptr);QVariantList roots()const{return top;}QVariantList items()const{return current;}
 void setSource(QString service,QString path);
 Q_INVOKABLE void select(int id);Q_INVOKABLE void trigger(int id);Q_INVOKABLE void back();
signals:void changed();void activated();
private slots:void reload(){load();}
private:void load();void flatten(const MenuLayout&,int depth=0);QString service,path;QVariantList top,current,all;int selected=0;uint generation=0;
};
