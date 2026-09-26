#include <QtTest>
#include <QQuickView>
#include <QQuickItem>
#include <QQmlContext>
#include <QQuickStyle>
#include <QTemporaryDir>
#include "core/Preferences.h"
class MenuServices:public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantMap state MEMBER state NOTIFY changed)
 Q_PROPERTY(bool busy MEMBER busy CONSTANT)
 Q_PROPERTY(QString message MEMBER message CONSTANT)
public: QVariantMap state{{"bluetoothAvailable",true},{"bluetoothPowered",false},{"bluetoothDevices",QVariantList{}},{"canReboot",true},{"harborSession",true},{"userDisplayName","Abdullah"}};bool busy=false;QString message,page,actionName;int logouts=0;
 Q_INVOKABLE void open(QString p){page=p;}
 Q_INVOKABLE void dismiss(){}
 Q_INVOKABLE void logout(){++logouts;}
 Q_INVOKABLE void action(QString n,QVariant={}){actionName=n;if(n=="bluetooth-scan"){state["bluetoothBusy"]=true;emit changed();}}
 Q_INVOKABLE void openTool(QString){}
 Q_INVOKABLE void refreshRecent(){}
 Q_INVOKABLE void startDiscovery(){}
 Q_INVOKABLE void stopDiscovery(){}
 Q_INVOKABLE void setPowered(bool){}
signals:void changed();
};
QQuickItem* item(QQuickItem* root,QString n){if(root->objectName()==n)return root;for(auto c:root->childItems())if(auto r=item(c,n))return r;return nullptr;}
class PanelMenusTest:public QObject{
 Q_OBJECT
private slots:
 void initTestCase(){QQuickStyle::setStyle("Basic");}
 void bluetoothPowerUpdatesWithoutBindingLoop(){
 QTemporaryDir dir;Preferences prefs(dir.filePath("prefs"));MenuServices services;QQuickView view;view.rootContext()->setContextProperty("Prefs",&prefs);view.rootContext()->setContextProperty("UI",&services);view.rootContext()->setContextProperty("System",&services);view.setSource(QUrl("qrc:/qml/BluetoothMenu.qml"));QVERIFY(view.status()==QQuickView::Ready);
 QTest::failOnWarning(QRegularExpression(".*Binding loop.*"));services.state["bluetoothPowered"]=true;emit services.changed();QTRY_VERIFY(view.rootObject()->property("powered").toBool());
 }
 void menuNavigationAndLogoutConfirmation(){
 QTemporaryDir dir;Preferences prefs(dir.filePath("prefs"));MenuServices services;
 QQuickView view;for(auto name:{"UI","System","Bluetooth"})view.rootContext()->setContextProperty(name,&services);view.rootContext()->setContextProperty("Prefs",&prefs);view.resize(380,580);view.setResizeMode(QQuickView::SizeRootObjectToView);view.setSource(QUrl("qrc:/qml/HarborMenu.qml"));QVERIFY(view.status()==QQuickView::Ready);view.show();
 auto about=item(view.rootObject(),"harbor-about");QVERIFY(about);QMetaObject::invokeMethod(about,"clicked");QCOMPARE(services.page,QString("settings:About"));
 auto logout=item(view.rootObject(),"harbor-logout");QVERIFY(logout);QMetaObject::invokeMethod(logout,"clicked");QCOMPARE(services.logouts,0);auto dialog=view.rootObject()->findChild<QObject*>("harbor-confirm");QVERIFY(dialog);QMetaObject::invokeMethod(dialog,"reject");QCOMPARE(services.logouts,0);QMetaObject::invokeMethod(logout,"clicked");QMetaObject::invokeMethod(dialog,"accept");QCOMPARE(services.logouts,1);
 }
};
QTEST_MAIN(PanelMenusTest)
#include "panel_menus_test.moc"
