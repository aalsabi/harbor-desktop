#include <QtTest>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include "core/DefaultApps.h"
class DefaultAppsTest:public QObject{
 Q_OBJECT
private slots:
 void rolesAndValidation(){DefaultApps apps;apps.refresh();QCOMPARE(apps.roles().size(),6);QVERIFY(!apps.setDefault("invalid","anything.desktop"));QVERIFY(!apps.setDefault("browser","../../bad.desktop"));}
 void isolatedAssociation(){DefaultApps apps;apps.refresh();QVariantMap pdf;for(const auto &entry:apps.roles())if(entry.toMap().value("key")=="pdf")pdf=entry.toMap();QCOMPARE(pdf.value("apps").toList().size(),1);QVERIFY(apps.setDefault("pdf","harbor-test.desktop"));for(const auto &entry:apps.roles())if(entry.toMap().value("key")=="pdf")QCOMPARE(entry.toMap().value("currentId").toString(),QString("harbor-test.desktop"));QVERIFY(!apps.setDefault("browser","harbor-test.desktop"));}
};
int main(int argc,char **argv){QTemporaryDir temp;if(!temp.isValid())return 2;auto base=temp.path();qputenv("XDG_CONFIG_HOME",(base+"/config").toUtf8());qputenv("XDG_CONFIG_DIRS",(base+"/etc").toUtf8());qputenv("XDG_DATA_HOME",(base+"/data").toUtf8());qputenv("XDG_DATA_DIRS",(base+"/share").toUtf8());QDir().mkpath(base+"/config");QDir().mkpath(base+"/data/applications");QFile desktop(base+"/data/applications/harbor-test.desktop");if(!desktop.open(QIODevice::WriteOnly))return 2;desktop.write("[Desktop Entry]\nType=Application\nName=Harbor test viewer\nExec=/bin/true %f\nMimeType=application/pdf;\n");desktop.close();QFile cache(base+"/data/applications/mimeinfo.cache");if(!cache.open(QIODevice::WriteOnly))return 2;cache.write("[MIME Cache]\napplication/pdf=harbor-test.desktop;\n");cache.close();QCoreApplication app(argc,argv);DefaultAppsTest test;return QTest::qExec(&test,argc,argv);}
#include "default_apps_test.moc"
