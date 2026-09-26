#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include "core/Applications.h"
class AppsTest : public QObject {
    Q_OBJECT
    QTemporaryDir data;
private slots:
    void initTestCase() {
        qputenv("XDG_DATA_HOME", data.path().toUtf8());
        QDir().mkpath(data.path() + "/applications");
        QFile f(data.path() + "/applications/harbor-icon-test.desktop");
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(
            ("[Desktop Entry]\nType=Application\nName=Harbor Icon Test\nExec=/bin/true\nStartupWMClass=HarborIconFixture\nIcon=" +
             data.path() + "/official icon.png\n")
                .toUtf8());
    }
    void absoluteIconsArePreserved() {
        Applications a;
        for (auto v : a.entries()) {
            auto entry = v.toMap();
            if (entry["id"] == "harbor-icon-test.desktop") {
                QCOMPARE(entry["icon"].toString(), data.path() + "/official icon.png");
                return;
            }
        }
        QFAIL("fixture app missing");
    }

    void runningWindowUsesDesktopEntryIcon() {
        Applications a;
        QString icon;
        QVERIFY(QMetaObject::invokeMethod(&a, "iconForAppId", Q_RETURN_ARG(QString, icon),
                                          Q_ARG(QString, QString("HarborIconFixture"))));
        QCOMPARE(icon, data.path() + "/official icon.png");
        QCOMPARE(a.iconForAppId("harbor-icon-test"), icon);
        QCOMPARE(a.iconForAppId("harbor-icon-test.desktop"), icon);
        QCOMPARE(a.iconForAppId("harboriconfixture"), icon);
    }
    void hiddenEntriesStayHidden() {
        QFile f(data.path() + "/applications/harbor-hidden-test.desktop");
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("[Desktop Entry]\nType=Application\nName=Harbor Hidden Test\nExec=/bin/true\nHidden=true\n");
        f.close();
        Applications a;
        for (auto v : a.entries())
            QVERIFY(v.toMap()["id"] != "harbor-hidden-test.desktop");
    }
    void hiddenPreferencesAreNotDefaultTerminal() {
        QTemporaryDir tools;
        QFile executable(tools.path() + "/fixture-terminal");
        QVERIFY(executable.open(QIODevice::WriteOnly));
        executable.write("#!/bin/sh\nexit 0\n");
        executable.close();
        QVERIFY(executable.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        QVERIFY(QFile::link(executable.fileName(), tools.path() + "/x-terminal-emulator"));
        QFile hidden(data.path() + "/applications/harbor-terminal-preferences.desktop");
        QVERIFY(hidden.open(QIODevice::WriteOnly));
        hidden.write(("[Desktop Entry]\nType=Application\nName=Terminal preferences\nExec=" +
                      executable.fileName() + " --preferences\nNoDisplay=true\n")
                         .toUtf8());
        hidden.close();
        auto old = qgetenv("PATH");
        qputenv("PATH", tools.path().toUtf8());
        Applications apps;
        const auto selected = apps.desktopForTool("terminal");
        qputenv("PATH", old);
        QVERIFY(selected != "harbor-terminal-preferences.desktop");
    }
    void pathsCannotBeLaunchedAsIds() {
        Applications a;
        QSignalSpy errors(&a, &Applications::error);
        QVERIFY(!a.launch("../../untrusted.desktop"));
        QCOMPARE(errors.size(), 1);
    }
};
QTEST_GUILESS_MAIN(AppsTest)
#include "apps_test.moc"
