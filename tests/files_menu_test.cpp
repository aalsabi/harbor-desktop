#include <QtTest>
#include <QDBusConnection>
#include "core/FilesMenu.h"
class FilesMenuTest:public QObject {
 Q_OBJECT
private slots:
 void panelCanReadAndActivateFilesMenu(){
  FilesMenu exporter;GlobalMenu reader;QSignalSpy actions(&exporter,&FilesMenu::action);
  reader.setSource(QDBusConnection::sessionBus().baseService(),exporter.objectPath());
  QTRY_COMPARE(reader.roots().size(),5);
  QCOMPARE(reader.roots().at(0).toMap()["label"].toString(),QString("File"));
  reader.select(100);reader.trigger(101);QTRY_COMPARE(actions.size(),1);QCOMPARE(actions.takeFirst()[0].toString(),QString("newTab"));
  reader.select(200);reader.trigger(201);QTest::qWait(50);QCOMPARE(actions.size(),0); // No selection.
  exporter.setState({{"selectionCount",1}});
  QTRY_VERIFY(reader.items().at(0).toMap()["enabled"].toBool());reader.trigger(201);QTRY_COMPARE(actions.size(),1);QCOMPARE(actions.takeFirst()[0].toString(),QString("copy"));
  exporter.setState({{"selectionCount",1},{"busy",true}});
  exporter.Event(105,"clicked",QDBusVariant(0),0);QCOMPARE(actions.size(),0); // Rename while busy.
  exporter.Event(999,"clicked",QDBusVariant(0),0);QCOMPARE(actions.size(),0);
 }
};
QTEST_GUILESS_MAIN(FilesMenuTest)
#include "files_menu_test.moc"
