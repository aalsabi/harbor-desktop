#include <QtTest>
#include <QDBusContext>
#include <QTimeZone>
#include <QDBusConnection>
#include <QDBusMessage>
#include "core/DateTimeSettings.h"
class ClockService : public QObject {
 Q_OBJECT
 Q_CLASSINFO("D-Bus Interface", "org.freedesktop.timedate1")
 Q_PROPERTY(QString Timezone READ timezone)
 Q_PROPERTY(bool NTP READ ntp)
 Q_PROPERTY(bool CanNTP READ canNtp)
 Q_PROPERTY(bool NTPSynchronized READ ntp)
public:
 QString zone="Etc/UTC"; bool automatic=false; qint64 received=0; bool interactive=false; bool relative=true;
 QString timezone()const{return zone;} bool ntp()const{return automatic;} bool canNtp()const{return true;}
public slots:
 void SetTimezone(const QString &value,bool auth){zone=value;interactive=auth;}
 void SetNTP(bool value,bool auth){automatic=value;interactive=auth;}
 void SetTime(qlonglong value,bool rel,bool auth){received=value;relative=rel;interactive=auth;}
};
class DateTimeTest : public QObject {
 Q_OBJECT
private slots:
 void validation(){
  qint64 value=0;
  QVERIFY(DateTimeSettings::parseDateTime("2026-09-24","12:34:56","Etc/UTC",&value));
  QCOMPARE(value,QDateTime(QDate(2026,9,24),QTime(12,34,56),QTimeZone("Etc/UTC")).toMSecsSinceEpoch()*1000);
  QVERIFY(!DateTimeSettings::parseDateTime("2026-03-29","02:30:00","Europe/Paris",nullptr));
  QVERIFY(!DateTimeSettings::parseDateTime("1969-01-01","12:00:00","Etc/UTC",nullptr));
  QVERIFY(!DateTimeSettings::parseDateTime("2026-2-28","12:00:00","Etc/UTC",nullptr));
  QVERIFY(!DateTimeSettings::parseDateTime("2026-02-30","12:34:56","Etc/UTC",nullptr));
  QVERIFY(!DateTimeSettings::parseDateTime("2026-02-28","24:00:00","Etc/UTC",nullptr));
  QVERIFY(!DateTimeSettings::parseDateTime("2026-02-28","12:00:00","Not/AZone",nullptr));
 }
 void isolatedService(){
  auto bus=QDBusConnection::sessionBus(); QVERIFY(bus.isConnected()); ClockService fake;
  QVERIFY(bus.registerService("org.freedesktop.timedate1"));
  QVERIFY(bus.registerObject("/org/freedesktop/timedate1",&fake,QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllProperties));
  DateTimeSettings settings(bus); QVERIFY(!settings.available()); settings.refresh();
  QTRY_VERIFY(settings.available()); QVERIFY(!settings.busy()); QCOMPARE(settings.timezone(),QString("Etc/UTC"));
  settings.setTimezone("Europe/Paris"); QCOMPARE(settings.timezone(),QString("Etc/UTC")); QTRY_VERIFY(!settings.busy()); QCOMPARE(settings.timezone(),QString("Europe/Paris")); QVERIFY(fake.interactive);
  settings.setTimezone("../../invalid"); QVERIFY(!settings.error().isEmpty()); QCOMPARE(fake.zone,QString("Europe/Paris"));
  settings.setNtp(true); QTRY_VERIFY(!settings.busy()); QVERIFY(settings.ntp()); settings.setDateTime("2026-09-24","12:34:56"); QCOMPARE(fake.received,qint64(0));
  settings.setNtp(false); QTRY_VERIFY(!settings.busy()); settings.setDateTime("2026-09-24","12:34:56"); QTRY_VERIFY(!settings.busy()); QVERIFY(fake.received>0); QVERIFY(!fake.relative); QVERIFY(fake.interactive);
  bus.unregisterObject("/org/freedesktop/timedate1"); bus.unregisterService("org.freedesktop.timedate1");
 }
};
QTEST_GUILESS_MAIN(DateTimeTest)
#include "date_time_test.moc"
