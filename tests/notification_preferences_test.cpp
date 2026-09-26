#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QSaveFile>
#include "core/Notifications.h"
class PolicyTest : public QObject {
    Q_OBJECT
private slots:
    void externalProfileChangeSurvivesImmediateSave() {
        QTemporaryDir tmp;
        auto path = tmp.filePath("prefs.ini");
        NotificationPreferences prefs(path);
        QVariantMap p{{"name", "Original"}, {"start", "09:00"}, {"end", "17:00"}};
        QVERIFY(prefs.saveProfile(p));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        auto bytes = f.readAll();
        f.close();
        QVERIFY(bytes.contains("Original"));
        bytes.replace("Original", "Externally renamed");
        QSaveFile external(path);
        QVERIFY(external.open(QIODevice::WriteOnly));
        QCOMPARE(external.write(bytes), bytes.size());
        QVERIFY(external.commit());
        p["name"] = "Second";
        QVERIFY(prefs.saveProfile(p));
        QCOMPARE(prefs.profiles().size(), 2);
        QCOMPARE(prefs.profiles()[0].toMap()["name"].toString(), QString("Externally renamed"));
    }
    void scheduleBoundaries() {
        QVariantMap p{{"days", QVariantList{1}}, {"start", "09:00"}, {"end", "17:00"}, {"scheduled", true}};
        auto monday = QDate(2026, 9, 21);
        QVERIFY(!NotificationPreferences::scheduleMatches(p, QDateTime(monday, QTime(8, 59, 59))));
        QVERIFY(NotificationPreferences::scheduleMatches(p, QDateTime(monday, QTime(9, 0))));
        QVERIFY(!NotificationPreferences::scheduleMatches(p, QDateTime(monday, QTime(17, 0))));
        QVERIFY(!NotificationPreferences::scheduleMatches(p, QDateTime(monday.addDays(1), QTime(10, 0))));
        p["days"] = QVariantList{7};
        p["start"] = "22:00";
        p["end"] = "07:00";
        QVERIFY(NotificationPreferences::scheduleMatches(p, QDateTime(monday, QTime(6, 59, 59))));
        QVERIFY(!NotificationPreferences::scheduleMatches(p, QDateTime(monday, QTime(7, 0))));
        p["scheduled"] = false;
        QVERIFY(!NotificationPreferences::scheduleMatches(p, QDateTime(monday, QTime(6, 0))));
    }
    void overlappingProfilesAndRestart() {
        QTemporaryDir tmp;
        auto path = tmp.filePath("prefs.ini");
        NotificationPreferences prefs(path);
        auto now = QTime::currentTime();
        QVariantMap p{{"name", "First"},
                      {"start", now.addSecs(-120).toString("HH:mm")},
                      {"end", now.addSecs(120).toString("HH:mm")},
                      {"scheduled", true},
                      {"days", QVariantList{1, 2, 3, 4, 5, 6, 7}},
                      {"allowed", QStringList{"first"}}};
        QVERIFY(prefs.saveProfile(p));
        p["name"] = "Second";
        p["allowed"] = QStringList{"second"};
        QVERIFY(prefs.saveProfile(p));
        QCOMPARE(prefs.activeProfile()["name"].toString(), QString("First"));
        QVERIFY(prefs.attentionAllowed("first"));
        QVERIFY(!prefs.attentionAllowed("second"));
        auto second = prefs.profiles()[1].toMap()["id"].toString();
        prefs.setFocusMode(second);
        QVERIFY(prefs.attentionAllowed("second"));
        QVERIFY(!prefs.attentionAllowed("first"));
        NotificationPreferences restarted(path);
        QCOMPARE(restarted.focusMode(), second);
        QVERIFY(restarted.attentionAllowed("second"));
        prefs.setDoNotDisturb(true);
        QVERIFY(!prefs.attentionAllowed("second"));
        prefs.setDoNotDisturb(false);
        prefs.setFocusMode("auto");
        QCOMPARE(prefs.activeProfile()["name"].toString(), QString("First"));
    }
    void scheduledFocus() {
        QVariantMap p{{"days", QVariantList{1}}, {"start", "22:00"}, {"end", "07:00"}, {"scheduled", true}};
        QVERIFY(NotificationPreferences::scheduleMatches(p, QDateTime(QDate(2026, 9, 21), QTime(23, 0))));
        QVERIFY(NotificationPreferences::scheduleMatches(p, QDateTime(QDate(2026, 9, 22), QTime(6, 0))));
        QVERIFY(!NotificationPreferences::scheduleMatches(p, QDateTime(QDate(2026, 9, 22), QTime(23, 0))));
        QVERIFY(!NotificationPreferences::scheduleMatches(p, QDateTime(QDate(2026, 9, 22), QTime(7, 0))));
    }
    void profilesAndManualOverride() {
        QTemporaryDir tmp;
        auto path = tmp.filePath("prefs.ini");
        NotificationPreferences prefs(path);
        QVariantMap p{{"name", "Work"},     {"days", QVariantList{1, 2, 3, 4, 5}},
                      {"start", "09:00"},   {"end", "17:00"},
                      {"scheduled", false}, {"allowed", QStringList{"mail.desktop"}}};
        QVERIFY(prefs.saveProfile(p));
        auto id = prefs.profiles()[0].toMap()["id"].toString();
        prefs.setFocusMode(id);
        QVERIFY(prefs.attentionAllowed("mail.desktop"));
        QVERIFY(!prefs.attentionAllowed("chat.desktop"));
        prefs.setFocusMode("off");
        QVERIFY(prefs.attentionAllowed("chat.desktop"));
        prefs.setDoNotDisturb(true);
        QVERIFY(!prefs.attentionAllowed("mail.desktop"));
        prefs.setDoNotDisturb(false);
        prefs.setFocusMode(id);
        NotificationPreferences other(path);
        QCOMPARE(other.focusMode(), id);
        prefs.removeProfile(id);
        QCOMPARE(prefs.focusMode(), QString("auto"));
        QVERIFY(!prefs.saveProfile({{"name", "Bad"}, {"start", "99:99"}, {"end", "17:00"}}));
    }
    void focusRetainsHistory() {
        QTemporaryDir tmp;
        auto path = tmp.filePath("prefs.ini");
        Notifications server(false, nullptr, path);
        NotificationPreferences prefs(path);
        server.Notify("Chat", 0, "", "Hello", "Body", {}, {{"desktop-entry", "chat.desktop"}}, 0);
        QVERIFY(prefs.saveProfile(
            {{"name", "Quiet"}, {"scheduled", false}, {"start", "09:00"}, {"end", "17:00"}}));
        prefs.setFocusMode(prefs.profiles()[0].toMap()["id"].toString());
        QTRY_COMPARE(server.attentionCount(), 0);
        QCOMPARE(server.items().size(), 1);
        prefs.setFocusMode("off");
        QTRY_COMPARE(server.attentionCount(), 1);
    }

    void sharedPolicy() {
        QTemporaryDir tmp;
        auto path = tmp.filePath("prefs.ini");
        Notifications server(false, nullptr, path);
        NotificationPreferences settings(path);
        server.Notify("Chat", 0, "", "Secret", "Private", {}, {{"desktop-entry", "chat.desktop"}}, 0);
        QTRY_COMPARE(settings.applications().size(), 1);
        QCOMPARE(server.attentionCount(), 1);
        settings.setDoNotDisturb(true);
        QTRY_COMPARE(server.attentionCount(), 0);
        QCOMPARE(server.items().size(), 1);
        settings.setPreview("chat.desktop", false);
        QTRY_COMPARE(server.items()[0].toMap()["body"].toString(), QString());
        settings.setEnabled("chat.desktop", false);
        QTRY_COMPARE(server.items().size(), 0);
        server.Notify("Chat", 0, "", "Hidden", "Hidden", {}, {{"desktop-entry", "chat.desktop"}}, 0);
        QCOMPARE(server.items().size(), 0);
        settings.setEnabled("chat.desktop", true);
        settings.setDoNotDisturb(false);
        QTest::qWait(30);
        server.Notify("Chat", 0, "", "New", "Preview hidden", {}, {{"desktop-entry", "chat.desktop"}}, 0);
        QTRY_COMPARE(server.items().size(), 1);
        QCOMPARE(server.items()[0].toMap()["body"].toString(), QString());
    }
};
QTEST_GUILESS_MAIN(PolicyTest)
#include "notification_preferences_test.moc"
