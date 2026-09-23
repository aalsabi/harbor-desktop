#include "Command.h"
#include <QProcessEnvironment>
Command::Command(QObject* parent):QObject(parent){timer.setSingleShot(true);
 connect(&timer,&QTimer::timeout,this,[this]{timedOut=true;process.kill();});
 connect(&process,&QProcess::errorOccurred,this,[this](QProcess::ProcessError e){if(e==QProcess::FailedToStart)finish(false,process.errorString());});
 connect(&process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus status){QString out=QString::fromUtf8(process.readAllStandardOutput());QString err=QString::fromUtf8(process.readAllStandardError());finish(!timedOut&&code==0&&status==QProcess::NormalExit,timedOut?tr("Operation timed out"):code==0?out:err.isEmpty()?tr("Operation failed"):err);});
}
void Command::run(const QString& program,const QStringList& args,int timeout){if(active)return;active=true;timedOut=false;emit busyChanged();auto env=QProcessEnvironment::systemEnvironment();env.insert("LC_ALL","C.UTF-8");process.setProcessEnvironment(env);process.start(program,args);timer.start(timeout);}
void Command::finish(bool ok,QString text){if(!active)return;timer.stop();active=false;emit busyChanged();emit finished(ok,text.trimmed());}
