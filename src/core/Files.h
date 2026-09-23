#pragma once
#include <QObject>
#include <QVariantList>
#include <QFileSystemWatcher>
class Files: public QObject {
 Q_OBJECT
 Q_PROPERTY(QString path READ path NOTIFY changed)
 Q_PROPERTY(QVariantList entries READ entries NOTIFY changed)
 Q_PROPERTY(QVariantList places READ places NOTIFY changed)
 Q_PROPERTY(QStringList tabs READ tabs NOTIFY changed)
 Q_PROPERTY(int tab READ tab NOTIFY changed)
 Q_PROPERTY(bool hidden READ hidden WRITE setHidden NOTIFY changed)
 Q_PROPERTY(QString search READ search WRITE setSearch NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
public:
 explicit Files(QObject* parent=nullptr);
 QString path()const;QVariantList entries()const;QVariantList places()const;
 QStringList tabs()const{return locations;}int tab()const{return active;}
 bool hidden()const{return showHidden;}QString search()const{return filter;}QString error()const{return message;}bool busy()const{return working;}
 void setHidden(bool);void setSearch(QString);
 Q_INVOKABLE void navigate(QString);
 Q_INVOKABLE void goHome();
 Q_INVOKABLE void back();Q_INVOKABLE void forward();Q_INVOKABLE void up();
 Q_INVOKABLE void addTab();Q_INVOKABLE void selectTab(int);Q_INVOKABLE void closeTab(int);
 Q_INVOKABLE QVariantMap preview(QString);
 Q_INVOKABLE QVariantList children(QString);
 Q_INVOKABLE QString parentPath(QString);
 Q_INVOKABLE void open(QString);
 Q_INVOKABLE void operate(QString action,QStringList sources,QString name={});
 Q_INVOKABLE void copy(QStringList sources,bool cut=false);
 Q_INVOKABLE void paste();
 Q_INVOKABLE void drop(QVariantList urls,bool move);
 Q_INVOKABLE void refresh();
signals:void changed();
private:
 QStringList locations;int active=0;bool showHidden=false,working=false;QString filter,message;
 QList<QStringList> histories;QList<int> positions;QStringList clipboard;bool cutting=false;
 QFileSystemWatcher watcher;void watch();void go(QString,bool record);QVariantList list(QString,bool)const;
};
