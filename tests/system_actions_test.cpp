#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include "core/SystemServices.h"
class SystemActionsTest : public QObject {
    Q_OBJECT
private slots:
    void lastSliderValueIsNotLostWhileBusy() {
        QTemporaryDir dir;
        auto old = qgetenv("PATH");
        qputenv("PATH", dir.path().toUtf8());
        QFile executable(dir.filePath("wpctl"));
        QVERIFY(executable.open(QIODevice::WriteOnly));
        executable.write(
            "#!/usr/bin/python3\nimport sys,time,pathlib\nif sys.argv[1]=='set-volume':\n with pathlib.Path(__file__).with_name('calls').open('a') as f:f.write(sys.argv[-1]+'\\n')\n time.sleep(.15)\nelse:print('Volume: 0.5')\n");
        executable.close();
        QVERIFY(executable.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        {
            SystemServices system(nullptr, false);
            system.action("volume", .1);
            system.action("volume", .3);
            system.action("volume", .8);
            QTRY_VERIFY(!system.busy());
            QTRY_VERIFY(([&] {
                for (auto command : system.findChildren<Command*>())
                    if (command->busy())
                        return false;
                return true;
            })());
            QFile calls(dir.filePath("calls"));
            QVERIFY(calls.open(QIODevice::ReadOnly));
            auto text = calls.readAll();
            qputenv("PATH", old);
            QCOMPARE(text, QByteArray("0.1\n0.8\n"));
        }
        qputenv("PATH", old);
    }
};
QTEST_GUILESS_MAIN(SystemActionsTest)
#include "system_actions_test.moc"
