#pragma once
#include <QObject>
#include <QProcess>
#include <QTimer>
class Command:public QObject {
 Q_OBJECT
 Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
public:explicit Command(QObject* parent=nullptr);bool busy()const{return active;}
 void run(const QString& program,const QStringList& args,int timeout=5000);
signals:void finished(bool ok,QString output);void busyChanged();
private:void finish(bool ok,QString text);QProcess process;QTimer timer;bool active=false;bool timedOut=false;
};
