#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include "core/Region.h"
class RegionTest : public QObject {
    Q_OBJECT
private slots:
    void missingMeasurementIsVisible(){
        QTemporaryDir dir;Region r(dir.path());auto draft=r.defaults("ar_SA");draft["measurement"]="us";
        auto notice=Region::availabilityNotice(draft,{"ar_sa.utf8"});QVERIFY(notice.contains("measurement"));
        QVERIFY(Region::availabilityNotice(draft,{"ar_sa.utf8","en_us.utf8"}).isEmpty());
        draft["measurement"]="uk";QVERIFY(Region::availabilityNotice(draft,{"ar_sa.utf8"}).contains("measurement"));
        draft=r.defaults("en_US");draft["measurement"]="metric";QVERIFY(Region::availabilityNotice(draft,{"en_us.utf8"}).contains("measurement"));
        QVERIFY(Region::availabilityNotice(draft,{"en_us.utf8","fr_fr.utf8"}).isEmpty());
    }
    void draftAndPersistence(){
        QTemporaryDir dir; Region r(dir.path());auto original=r.state();auto draft=r.defaults("en_US");
        draft["region"]="fr_FR";r.preview(draft);QCOMPARE(r.state(),original);
        draft=r.defaults("fr_FR");draft["languages"]=QStringList{"fr","en","ar"};QVERIFY(r.apply(draft));
        Region loaded(dir.path());QCOMPARE(loaded.state().value("region").toString(),QString("fr_FR"));
        QCOMPARE(loaded.state().value("languages").toStringList(),QStringList({"fr","en","ar"}));
    }
    void invalidAndFailure(){
        QTemporaryDir dir; Region r(dir.path());auto valid=r.defaults("en_US");QVERIFY(r.apply(valid));auto original=r.state();
        auto invalid=valid;invalid["numberDecimal"]=invalid["numberGroup"];QVERIFY(!r.apply(invalid));QCOMPARE(r.state(),original);
        invalid=valid;invalid["dateFormats"]=QStringList{"unterminated '"};QVERIFY(!r.apply(invalid));
        invalid=valid;invalid["currency"]="ZZZ";QVERIFY(!r.apply(invalid));
        QFile blocker(dir.path()+"/file");QVERIFY(blocker.open(QIODevice::WriteOnly));blocker.close();
        Region fail(dir.path()+"/file");auto before=fail.state();QVERIFY(!fail.apply(valid));QCOMPARE(fail.state(),before);
    }
    void formats(){
        QTemporaryDir dir;Region r(dir.path());auto d=r.defaults("en_US");
        d["numberGroup"]=" ";d["numberDecimal"]=",";d["currencyGroup"]=".";d["currencyDecimal"]=",";
        d["dateFormats"]=QStringList{"yyyy-MM-dd","yyyy","MM","dd"};
        d["timeFormats"]=QStringList{"HH:mm","HH:mm:ss","h AP","h AP"};d["pm"]="evening";
        d["firstDay"]=1;d["measurement"]="metric";
        auto p=r.preview(d);QVERIFY(!p.contains("error"));QCOMPARE(p["date"].toString(),QString("2026-09-23"));
        QCOMPARE(p["time"].toString(),QString("17:08"));QCOMPARE(p["number"].toString(),QString("1 234 567,89"));
        QCOMPARE(p["currency"].toString(),QString("USD 1.234.567,89"));QCOMPARE(p["times"].toStringList().at(2),QString("5 evening"));
        QCOMPARE(p["weekdays"].toStringList().first(),QString("Monday"));QVERIFY(p["measurementExample"].toString().contains("km"));
        QVERIFY(r.apply(d));
        const QString expectedDate=QDate::currentDate().toString("yyyy-MM-dd");
        QVERIFY(r.clockText().startsWith(expectedDate+"  "));
        QVERIFY(QRegularExpression("^\\d{4}-\\d{2}-\\d{2}  \\d{2}:\\d{2}$").match(r.clockText()).hasMatch());
        d["firstDay"]=7;QCOMPARE(r.preview(d)["weekdays"].toStringList().first(),QString("Sunday"));
        d["timeFormats"]=QStringList{"'custom clock'","HH:mm:ss","h AP","h AP"};
        QVERIFY(r.apply(d));Region loaded(dir.path());QCOMPARE(loaded.clockText(),expectedDate+"  custom clock");
    }
    void generationPlans(){
        QTemporaryDir dir;Region r(dir.path());
        const QStringList supported{"en_US.UTF-8","en_GB.UTF-8","fr_FR.UTF-8","ar_SA.UTF-8"};
        auto draft=r.defaults("en_US");
        auto plan=Region::planLocales(draft,supported,{"C","C.utf8"});
        QCOMPARE(plan.value("locales").toStringList(),QStringList{"en_US.UTF-8"});
        QVERIFY(plan.value("error").toString().isEmpty());
        plan=Region::planLocales(draft,supported,{"en_US.utf8"});QVERIFY(plan.value("locales").toStringList().isEmpty());
        draft["languages"]=QStringList{"ar","en"};draft["measurement"]="metric";
        plan=Region::planLocales(draft,supported,{"en_US.utf8"});
        QCOMPARE(plan.value("locales").toStringList(),QStringList{"ar_SA.UTF-8"});
        draft["languages"]=QStringList{"en"};
        plan=Region::planLocales(draft,supported,{"en_US.utf8"});
        QCOMPARE(plan.value("locales").toStringList(),QStringList{"fr_FR.UTF-8"});
        draft["measurement"]="uk";plan=Region::planLocales(draft,supported,{"en_US.utf8"});
        QCOMPARE(plan.value("locales").toStringList(),QStringList{"en_GB.UTF-8"});
        draft["region"]="xx_YY";QVERIFY(!Region::planLocales(draft,supported,{}).value("error").toString().isEmpty());
        const auto before=r.state();QSignalSpy finished(&r,&Region::generationFinished);
        r.generateAndApply(draft);QCOMPARE(finished.size(),1);QCOMPARE(finished.first().first().toBool(),false);
        QCOMPARE(r.state(),before);QVERIFY(!r.busy());
    }
    void malformedPersistenceIsIgnored(){
        QTemporaryDir dir;Region original(dir.path());const auto safe=original.state();
        for(const auto &data:QList<QByteArray>{"not JSON", "[]", "{\"schema\":2,\"region\":\"fr_FR\"}", "{\"schema\":1,\"region\":\"invalid\"}"}) {
            QFile file(dir.path()+"/harbor/region.json");QVERIFY(file.open(QIODevice::WriteOnly|QIODevice::Truncate));file.write(data);file.close();
            Region loaded(dir.path());QCOMPARE(loaded.state(),safe);
        }
    }
    void everyDefaultIsValid(){
        QTemporaryDir dir;Region r(dir.path());QVERIFY(!r.currencies().isEmpty());
        for(const auto &region:r.regions()) {
            auto defaults=r.defaults(region.toMap().value("code").toString());
            auto preview=r.preview(defaults);
            QVERIFY2(!preview.contains("error"), qPrintable(defaults.value("region").toString()+": "+preview.value("error").toString()));
        }
        QVERIFY(!r.preview(r.defaults("invalid_locale")).contains("error"));
    }
    void externalUpdate(){
        QTemporaryDir dir;Region first(dir.path()),second(dir.path());auto d=first.defaults("fr_FR");QVERIFY(first.apply(d));
        QTRY_COMPARE(second.state().value("region").toString(),QString("fr_FR"));
    }
};
QTEST_GUILESS_MAIN(RegionTest)
#include "region_test.moc"
