#include <QtTest>
#include "core/Notifications.h"
class NotificationsTest: public QObject {
 Q_OBJECT
private slots:
 void replacementKeepsOneRow(){Notifications n(false);uint id=n.Notify("test",0,"","First","Body",{}, {},0);QCOMPARE(n.items().size(),1);QCOMPARE(n.Notify("test",id,"","Second","Body",{}, {},0),id);QCOMPARE(n.items().size(),1);QCOMPARE(n.items()[0].toMap()["summary"].toString(),QString("Second"));}
 void closeRemovesAndSignals(){Notifications n(false);uint id=n.Notify("test",0,"","One","",{}, {},0);QSignalSpy spy(&n,&Notifications::NotificationClosed);n.CloseNotification(id);QCOMPARE(n.items().size(),0);QCOMPARE(spy.size(),1);}
 void expiryIsHonored(){Notifications n(false);n.Notify("test",0,"","One","",{}, {},25);QTRY_COMPARE_WITH_TIMEOUT(n.items().size(),0,1000);}
};
QTEST_GUILESS_MAIN(NotificationsTest)
#include "notifications_test.moc"
