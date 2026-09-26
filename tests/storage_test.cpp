#include <QtTest>
#include "core/StorageSettings.h"
class StorageTest : public QObject {
 Q_OBJECT
private slots:
 void usageAndInvalidMount(){
  StorageSettings settings; QVERIFY(settings.volumes().isEmpty()); settings.refresh(); QTRY_VERIFY(!settings.busy());
  for(const auto &entry:settings.volumes()) { const auto v=entry.toMap(); QVERIFY(v.value("total").toLongLong()>0); QVERIFY(v.value("used").toLongLong()>=0); QVERIFY(v.value("available").toLongLong()>=0); QVERIFY(v.value("fraction").toDouble()>=0); QVERIFY(v.value("fraction").toDouble()<=1); }
  QVERIFY(!settings.openVolume("https://example.com")); QVERIFY(!settings.error().isEmpty()); QVERIFY(!settings.openVolume("/not-a-harbor-mount"));
 }
};
QTEST_GUILESS_MAIN(StorageTest)
#include "storage_test.moc"
