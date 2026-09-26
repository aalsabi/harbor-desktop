#pragma once
#include <QObject>
#include <QVariantList>
#include <QDBusConnection>
#include <functional>
class Accounts:public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantList users READ users NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
public:
 explicit Accounts(QObject *parent=nullptr);
 explicit Accounts(const QDBusConnection &connection,QObject *parent=nullptr);
 QVariantList users()const{return rows;}QString error()const{return problem;}bool busy()const{return working;}
 Q_INVOKABLE void refresh();
 Q_INVOKABLE void setRealName(QString path,QString name);
 Q_INVOKABLE void createUser(QString username,QString realName,int accountType,QString password);
 Q_INVOKABLE void deleteUser(QString path,bool removeFiles=false,bool confirmed=false);
 Q_INVOKABLE void setAccountType(QString path,int accountType,bool confirmed=false);
 Q_INVOKABLE void setPassword(QString path,QString password);
 static bool validUserName(const QString &name);
 static QString passwordHash(QString password);
signals:void changed();
private:
 void load(std::function<void()> after={});
 void call(const QString &path,const QString &interface,const QString &method,const QVariantList &args,std::function<void(const QDBusMessage &)> done={});
 void fail(QString text);QVariantMap user(const QString &path)const;QString guard(const QVariantMap &row,bool destructive)const;
 QDBusConnection bus;QVariantList rows;QString problem;bool working=false;
};
