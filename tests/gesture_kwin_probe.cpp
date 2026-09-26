#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDBusConnectionInterface>
#include <QDir>
#include "core/GestureSettings.h"

int main(int argc,char **argv){
 QCoreApplication app(argc,argv);
 if(qEnvironmentVariable("HARBOR_GESTURE_INTEGRATION_PRIVATE")!="1"||qEnvironmentVariable("HARBOR_PRIVATE_BUS")!="1")return 2;
 auto bus=QDBusConnection::sessionBus();auto pid=bus.interface()->servicePid("org.kde.KWin");
 if(!pid.isValid())return 3;
 QFile command("/proc/"+QString::number(pid.value())+"/cmdline");
 if(!command.open(QIODevice::ReadOnly)||!command.readAll().split('\0').contains("--virtual")){QTextStream(stderr)<<"Refusing a non-virtual compositor\n";return 4;}
 QTemporaryDir directory(qEnvironmentVariable("XDG_CONFIG_HOME")+"/gestures-probe-XXXXXX");GestureSettings settings(nullptr,bus,directory.path());int phase=0;QString firstScript;
 auto fail=[&](QString error){QTextStream(stderr)<<"Phase "<<phase<<": "<<error<<"\n";app.exit(1);};
 QTimer deadline;deadline.setSingleShot(true);deadline.setInterval(20000);QObject::connect(&deadline,&QTimer::timeout,&app,[&]{fail("Gesture integration timed out");});deadline.start();
 QObject::connect(&settings,&GestureSettings::changed,&app,[&]{
  if(settings.busy()||phase==0)return;
  if(!settings.error().isEmpty()){fail(settings.error());return;}
  if(phase==1){
   if(!settings.active()||!settings.enabled()){fail("Initial Ready handshake was not accepted");return;}
   auto files=QDir(directory.path()).entryList({"generated-*"},QDir::Dirs|QDir::NoDotAndDotDot);if(files.size()!=1){fail("Missing generated gesture file");return;}firstScript=files[0];
   phase=0;QTimer::singleShot(0,&app,[&]{phase=2;settings.apply({{"4-Left","settings"},{"3-Down","notifications"}},true);});
  }else if(phase==2){
   auto files=QDir(directory.path()).entryList({"generated-*"},QDir::Dirs|QDir::NoDotAndDotDot);
   if(!settings.active()||files.size()!=1||files[0]==firstScript){fail("Updated gesture handlers did not load from a fresh QML URL");return;}
   phase=0;QTimer::singleShot(0,&app,[&]{phase=3;settings.apply(settings.mappings(),false);});
  }else if(phase==3){
   if(settings.active()||settings.enabled()){fail("Custom gestures did not disable");return;}
   phase=0;QTimer::singleShot(0,&app,[&]{phase=4;settings.refresh();});
  }else if(phase==4){
   if(settings.active()){fail("KWin retained the disabled gesture script");return;}
   QTextStream(stdout)<<"{\"virtualKWin\":true,\"readyHandshake\":true,\"mappingReload\":true,\"unloaded\":true}\n";app.exit(0);
  }
 });
 QTimer::singleShot(0,&app,[&]{phase=1;settings.apply({{"3-Up","launcher"}},true);});
 return app.exec();
}
