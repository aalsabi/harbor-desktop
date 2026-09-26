#include <QGuiApplication>
#include <QWindow>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);
    if (QGuiApplication::platformName() != QStringLiteral("wayland") ||
        qEnvironmentVariable("HOME") != QStringLiteral("/home/harbor") ||
        !qEnvironmentVariableIsEmpty("DISPLAY") || !qEnvironmentVariableIsEmpty("DBUS_SESSION_BUS_ADDRESS") ||
        QFile::exists(QStringLiteral("/etc/passwd")) ||
        QFile::exists(QStringLiteral("/run/dbus/system_bus_socket")))
        return 2;
    QWindow window;
    window.setTitle(QStringLiteral("Harbor private isolation fixture"));
    window.resize(320, 180);
    window.show();
    QTimer poll;
    QObject::connect(&poll, &QTimer::timeout, &app, [&] {
        if (!window.isExposed())
            return;
        QFile marker(QDir::homePath() + QStringLiteral("/gui-probe.json"));
        if (!marker.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            app.exit(3);
            return;
        }
        const QByteArray evidence = QJsonDocument(QJsonObject{{QStringLiteral("wayland"), true},
                                                              {QStringLiteral("exposed"), true},
                                                              {QStringLiteral("isolatedHome"), true},
                                                              {QStringLiteral("hostBusAbsent"), true}})
                                        .toJson(QJsonDocument::Compact);
        if (marker.write(evidence) != evidence.size()) {
            app.exit(4);
            return;
        }
        marker.close();
        app.exit(0);
    });
    poll.start(50);
    QTimer::singleShot(10000, &app, [&] {
        app.exit(5);
    });
    return app.exec();
}
