#include <QtTest>
#include <QQuickView>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQmlContext>
#include <QTemporaryDir>
#include "core/Preferences.h"
class SettingsMock:public QObject{
 Q_OBJECT
 Q_PROPERTY(QVariantMap state MEMBER state NOTIFY changed)
 Q_PROPERTY(QVariantList users MEMBER users NOTIFY changed)
 Q_PROPERTY(bool busy MEMBER busy NOTIFY changed)
 Q_PROPERTY(QString message MEMBER message NOTIFY changed)
 Q_PROPERTY(QString error MEMBER message NOTIFY changed)
public:QVariantMap state;QVariantList users;bool busy=false;QString message;QStringList applied;
 Q_INVOKABLE void windowAction(QString){}
 Q_INVOKABLE void apply(QStringList layouts,QString){applied=layouts;}
 Q_INVOKABLE void refresh(){}
 Q_INVOKABLE void switchNext(){}
signals:void changed();
};
QQuickItem* settingItem(QQuickItem* root,QString name){if(root->objectName()==name&&root->isVisible())return root;for(auto c:root->childItems())if(auto found=settingItem(c,name))return found;return nullptr;}
class SettingsUITest:public QObject{
 Q_OBJECT
private slots:
 void navigationAndKeyboard(){
  QQuickStyle::setStyle("Basic");QTemporaryDir temp;Preferences prefs(temp.filePath("prefs.ini"));SettingsMock system,accounts,keyboard,ui;
  keyboard.state={{"layouts",QStringList{"us"}},{"shortcut",""},{"catalog",QVariantList{QVariantMap{{"id","us"},{"name","English (US)"}},QVariantMap{{"id","ara"},{"name","Arabic"}},QVariantMap{{"id","fr"},{"name","French"}},QVariantMap{{"id","de"},{"name","German"}},QVariantMap{{"id","es"},{"name","Spanish"}}}}};
  QQuickView view;auto ctx=view.rootContext();ctx->setContextProperty("Prefs",&prefs);ctx->setContextProperty("System",&system);ctx->setContextProperty("Accounts",&accounts);ctx->setContextProperty("Keyboard",&keyboard);ctx->setContextProperty("UI",&ui);ctx->setContextProperty("HarborVersion","test");view.setResizeMode(QQuickView::SizeRootObjectToView);view.resize(960,720);view.setSource(QUrl("qrc:/qml/Settings.qml"));QVERIFY(view.status()==QQuickView::Ready);view.show();QTest::qWait(50);
  auto click=[&](QString name){auto item=settingItem(view.contentItem(),name);QVERIFY2(item,qPrintable(name));QTest::mouseClick(&view,Qt::LeftButton,Qt::NoModifier,item->mapToScene(QPointF(item->width()/2,item->height()/2)).toPoint());};
  auto search=settingItem(view.rootObject(),"settings-search");QVERIFY(search);search->setProperty("text","arabic");QTest::qWait(20);
  click("settings-nav-keyboard");QCOMPARE(view.rootObject()->property("section").toString(),QString("Keyboard"));
  click("edit-input-sources");QTest::qWait(30);
  click("add-input");QTest::qWait(30);
  auto sourceSearch=settingItem(view.contentItem(),"input-source-search");QVERIFY(sourceSearch);sourceSearch->setProperty("text","Arabic");QTest::qWait(20);
  click("input-candidate-ara");click("confirm-add-input");click("cancel-input-sources");QVERIFY(keyboard.applied.isEmpty());
  click("edit-input-sources");QTest::qWait(30);
  QVERIFY(!settingItem(view.contentItem(),"configured-input-ara"));
  click("add-input");QTest::qWait(30);click("input-candidate-ara");click("confirm-add-input");
  QVERIFY(settingItem(view.contentItem(),"configured-input-ara"));QVERIFY(keyboard.applied.isEmpty());
  click("add-input");QTest::qWait(20);
  auto duplicate=settingItem(view.contentItem(),"input-candidate-us");QVERIFY(duplicate);QVERIFY(!duplicate->isEnabled());
  click("input-candidate-fr");click("confirm-add-input");click("add-input");QTest::qWait(20);click("input-candidate-de");click("confirm-add-input");
  auto add=settingItem(view.contentItem(),"add-input");QVERIFY(add);QVERIFY(!add->isEnabled());
  click("remove-input");click("remove-input");
  if(qEnvironmentVariableIsSet("HARBOR_TEST_SCREENSHOTS"))view.grabWindow().save("/tmp/harbor-input-sources.png");
  click("done-input-sources");QCOMPARE(keyboard.applied,QStringList({"us","ara"}));
  search->setProperty("text",QString());
  view.rootObject()->setProperty("section","Appearance");QTest::qWait(30);click("appearance-dark");QVERIFY(prefs.dark());
  for(const auto& page:QStringList{"General","Wi-Fi","Bluetooth","Network","Sound","Appearance","Accessibility","Desktop & Dock","Displays","Keyboard","Battery","Users & Groups","About","Software Update"}){view.rootObject()->setProperty("section",page);QTest::qWait(10);QVERIFY(!view.grabWindow().isNull());}
  prefs.setLanguage("ar");view.rootObject()->setProperty("section","Keyboard");QTest::qWait(20);click("edit-input-sources");QTest::qWait(20);QVERIFY(!view.grabWindow().isNull());
  if(qEnvironmentVariableIsSet("HARBOR_TEST_SCREENSHOTS"))view.grabWindow().save("/tmp/harbor-input-sources-ar-dark.png");
  view.resize(740,560);QTest::qWait(30);
  if(qEnvironmentVariableIsSet("HARBOR_TEST_SCREENSHOTS"))view.grabWindow().save("/tmp/harbor-input-sources-ar-dark-small.png");
  auto done=settingItem(view.contentItem(),"done-input-sources");QVERIFY(done);auto doneBounds=done->mapRectToScene(done->boundingRect());QVERIFY(QRectF(0,0,view.width(),view.height()).contains(doneBounds));
  auto editorBackground=settingItem(view.contentItem(),"input-sources-background");QVERIFY(editorBackground);auto editorInterior=editorBackground->mapRectToScene(editorBackground->boundingRect()).adjusted(12,12,-12,-12);QVERIFY(editorInterior.contains(doneBounds));
  auto cancel=settingItem(view.contentItem(),"cancel-input-sources");QVERIFY(cancel);QVERIFY(editorInterior.contains(cancel->mapRectToScene(cancel->boundingRect())));
  auto remove=settingItem(view.contentItem(),"remove-input");QVERIFY(remove);QVERIFY(!remove->isEnabled());click("cancel-input-sources");
 }
};
QTEST_MAIN(SettingsUITest)
#include "settings_ui_test.moc"
