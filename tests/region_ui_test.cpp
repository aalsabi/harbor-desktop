#include <QtTest>
#include <QQuickView>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQmlContext>
#include <QTemporaryDir>
#include <QJSValue>
#include <QKeyEvent>
#include "core/Preferences.h"
#include "core/Region.h"
class RegionUIMock:public QObject{
 Q_OBJECT
 Q_PROPERTY(QVariantMap state MEMBER state NOTIFY changed)
 Q_PROPERTY(QVariantList users MEMBER users NOTIFY changed)
 Q_PROPERTY(bool busy MEMBER busy NOTIFY changed)
 Q_PROPERTY(QString message MEMBER message NOTIFY changed)
 Q_PROPERTY(QString error MEMBER message NOTIFY changed)
public:QVariantMap state;QVariantList users;bool busy=false;QString message;
 Q_INVOKABLE void windowAction(QString){} Q_INVOKABLE void refresh(){}
signals:void changed();
};
class RegionForUI:public Region {
 Q_OBJECT
 Q_PROPERTY(bool busy MEMBER testBusy NOTIFY changed)
 Q_PROPERTY(QString message MEMBER testMessage NOTIFY changed)
public:
 using Region::Region;
 QString testMessage;bool missing=false,testBusy=false;int generationCalls=0;QVariantMap snapshot;
 Q_INVOKABLE QVariantMap generationPlan(QVariantMap){return {{"locales",missing?QStringList{"ar_SA.UTF-8"}:QStringList{}},{"error",QString()}};}
 Q_INVOKABLE void generateAndApply(QVariantMap value){if(testBusy)return;++generationCalls;snapshot=value;testBusy=true;emit changed();}
 void finish(bool success){testBusy=false;testMessage=success?QString():QString("Authentication cancelled");emit changed();if(success)success=Region::apply(snapshot);emit generationFinished(success);}
};
static QQuickItem* item(QQuickItem* r,const QString& name){if(r->objectName()==name&&r->isVisible())return r;for(auto c:r->childItems())if(auto f=item(c,name))return f;return nullptr;}
class RegionUITest:public QObject {
 Q_OBJECT
private slots:
 void initTestCase(){QQuickStyle::setStyle("Basic");}
 void editCancelAndApply(){
  QTemporaryDir temp;Preferences prefs(temp.filePath("settings.ini"));RegionForUI region(temp.path());RegionUIMock mock;
  QQuickView view;auto ctx=view.rootContext();ctx->setContextProperty("Prefs",&prefs);ctx->setContextProperty("Region",&region);for(auto n:{"System","Accounts","Keyboard","UI"})ctx->setContextProperty(n,&mock);ctx->setContextProperty("HarborVersion","test");view.setResizeMode(QQuickView::SizeRootObjectToView);view.resize(960,800);view.setSource(QUrl("qrc:/qml/Settings.qml"));QVERIFY(view.status()==QQuickView::Ready);view.rootObject()->setProperty("section","Language & Region");view.show();QTest::qWait(150);
  auto page=item(view.contentItem(),"languageRegionPage");QVERIFY(page);
  auto before=region.state();
  auto shot=[&](QString name){if(qEnvironmentVariableIsSet("HARBOR_TEST_SCREENSHOTS"))view.grabWindow().save("/tmp/harbor-region-"+name+".png");};
  auto click=[&](QString name){auto b=item(view.contentItem(),name);if(!b)return false;QTest::mouseClick(&view,Qt::LeftButton,Qt::NoModifier,b->mapToScene(QPointF(b->width()/2,b->height()/2)).toPoint());QTest::qWait(30);return true;};
  auto type=[&](QString text){QKeyEvent event(QEvent::KeyPress,0,Qt::NoModifier,text);QCoreApplication::sendEvent(&view,&event);};
  shot("main");
  auto draft=[&]{return page->property("draft").value<QJSValue>().toVariant().toMap();};
  QVERIFY(click("hour24"));QVERIFY(draft().value("hour24").toBool()!=before.value("hour24").toBool());QVERIFY(draft().value("timeFormats").toList().first().toString()!=before.value("timeFormats").toStringList().first());QCOMPARE(region.state(),before);
  QVERIFY(click("revertRegion"));
  QVERIFY(click("addLanguage"));QTest::qWait(50);
  auto add=page->findChild<QObject*>("addLanguageDialog");QVERIFY(add);add->setProperty("selectedCode","ar");shot("languages");QVERIFY(click("confirmAddLanguage"));QCOMPARE(draft().value("languages").toList().size(),2);QCOMPARE(region.state(),before);
  QVERIFY(QMetaObject::invokeMethod(page,"moveLanguage",Q_ARG(QVariant,QVariant(-1))));QCOMPARE(draft().value("languages").toList().first().toString(),QString("ar"));
  QVERIFY(click("revertRegion"));
  QVERIFY(QMetaObject::invokeMethod(page,"openAdvanced"));QTest::qWait(80);shot("general");
  auto field=item(view.contentItem(),"numberDecimal");QVERIFY(field);field->forceActiveFocus();QTest::keyClick(&view,Qt::Key_A,Qt::ControlModifier);type(";");QCOMPARE(page->property("advancedDraft").value<QJSValue>().toVariant().toMap().value("numberDecimal").toString(),QString(";"));
  QVERIFY(click("cancelAdvanced"));QCOMPARE(region.state(),before);
  QVERIFY(QMetaObject::invokeMethod(page,"openAdvanced"));QTest::qWait(50);field=item(view.contentItem(),"numberDecimal");QVERIFY(field);QCOMPARE(field->property("text").toString(),before.value("numberDecimal").toString());
  auto tabs=item(view.contentItem(),"advancedRegionTabs");QVERIFY(tabs);tabs->setProperty("currentIndex",1);QTest::qWait(50);shot("dates");
  auto date=item(view.contentItem(),"formatPattern0");QVERIFY(date);date->forceActiveFocus();QTest::keyClick(&view,Qt::Key_A,Qt::ControlModifier);type("yyyy-MM-dd");QTest::qWait(30);
  tabs->setProperty("currentIndex",2);QTest::qWait(50);shot("times");
  QVERIFY(click("acceptAdvanced"));QCOMPARE(region.state(),before);QVERIFY(QMetaObject::invokeMethod(page,"applyChanges"));QCOMPARE(region.state().value("dateFormats").toStringList().first(),QString("yyyy-MM-dd"));
  Region reloaded(temp.path());QCOMPARE(reloaded.state(),region.state());
  const auto savedState=region.state();
  auto pending=page->property("draft").value<QJSValue>().toVariant().toMap();pending["firstDay"]=3;page->setProperty("draft",pending);region.missing=true;
  QVERIFY(QMetaObject::invokeMethod(page,"applyChanges"));QTest::qWait(50);QVERIFY(item(view.contentItem(),"confirmGeneration"));QCOMPARE(region.generationCalls,0);QCOMPARE(region.state(),savedState);
  shot("generate");prefs.setLanguage("ar");view.resize(740,560);QTest::qWait(40);shot("generate-ar");for(auto name:{"cancelGeneration","confirmGeneration"}){auto b=item(view.contentItem(),name);QVERIFY(b);QVERIFY(QRectF(0,0,740,560).contains(b->mapRectToScene(b->boundingRect())));}prefs.setLanguage("en");view.resize(960,800);QTest::qWait(30);QVERIFY(click("cancelGeneration"));QCOMPARE(region.state(),savedState);
  QVERIFY(QMetaObject::invokeMethod(page,"applyChanges"));QTest::qWait(30);QVERIFY(click("confirmGeneration"));QCOMPARE(region.generationCalls,1);QVERIFY(!item(view.contentItem(),"confirmGeneration")->isEnabled());QCOMPARE(region.state(),savedState);
  region.finish(false);QTest::qWait(30);QCOMPARE(region.state(),savedState);QVERIFY(item(view.contentItem(),"generationError"));QVERIFY(item(view.contentItem(),"generationError")->property("text").toString().contains("cancelled"));
  QVERIFY(click("confirmGeneration"));QCOMPARE(region.generationCalls,2);region.finish(true);QTest::qWait(40);QCOMPARE(region.state().value("firstDay").toInt(),3);QVERIFY(!item(view.contentItem(),"confirmGeneration"));

  view.resize(740,560);prefs.setLanguage("ar");prefs.setDark(true);QTest::qWait(50);QVERIFY(QMetaObject::invokeMethod(page,"openAdvanced"));QTest::qWait(60);shot("ar-small");
  for(auto name:{"cancelAdvanced","acceptAdvanced"}){auto b=item(view.contentItem(),name);QVERIFY(b);QVERIFY(QRectF(0,0,740,560).contains(b->mapRectToScene(b->boundingRect())));}
 }
};
QTEST_MAIN(RegionUITest)
#include "region_ui_test.moc"
