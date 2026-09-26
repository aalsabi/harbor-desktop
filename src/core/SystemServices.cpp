#include "SystemServices.h"
#include "SystemState.h"
#include <QDBusConnectionInterface>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QSysInfo>
#include <QProcess>
#include <QTimer>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <cmath>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QUuid>
#include <memory>
#include <QXmlStreamReader>
#include <QDesktopServices>
#include <QUrl>
#include <pwd.h>
#include <unistd.h>
#include <algorithm>
SystemServices::SystemServices(QObject* p, bool poll) : QObject(p) {
    values["userName"] = qEnvironmentVariable("USER");
    values["osName"] = QSysInfo::prettyProductName();
    values["architecture"] = QSysInfo::currentCpuArchitecture();
    values["kernel"] = QSysInfo::kernelVersion();
    values["displayOutputs"] = QVariantList{};
    values["displayPending"] = false;
    values["displayChanging"] = false;
    for (auto key : {"wifi", "network", "networks", "volume", "audio", "bluetooth", "devices", "brightness",
                     "power", "displays"})
        values[QString(key) + "Available"] = false;
    auto user = getpwuid(getuid());
    values["userDisplayName"] =
        user ? QString::fromLocal8Bit(user->pw_gecos).section(',', 0, 0) : values["userName"];
    if (values["userDisplayName"].toString().isEmpty())
        values["userDisplayName"] = values["userName"];
    if (!poll)
        return;
    bluetooth = new Bluetooth(this);
    connect(bluetooth, &Bluetooth::changed, this, [this] {
        auto state = bluetooth->state();
        for (auto i = state.begin(); i != state.end(); ++i)
            values[i.key()] = i.value();
        emit changed();
    });
    QTimer::singleShot(0, this, &SystemServices::refresh);
    auto t = new QTimer(this);
    connect(t, &QTimer::timeout, this, &SystemServices::refresh);
    t->start(15000);
}
void SystemServices::query(QString key, QString program, QStringList args) {
    const uint gen = ++generations[key];
    auto update = [this, key](bool ok, QString out) {
        values[key + "Available"] = ok;
        values[key] = ok ? out : QString();
        const auto parsed = SystemState::parse(key, ok ? out : QString());
        for (auto it = parsed.begin(); it != parsed.end(); ++it)
            values[it.key()] = it.value();
        if (key == "displays")
            values["displayOutputs"] =
                QJsonDocument::fromJson(out.toUtf8()).object()["outputs"].toArray().toVariantList();
        emit changed();
    };
    if (QStandardPaths::findExecutable(program).isEmpty()) {
        update(false, {});
        return;
    }
    auto c = new Command(this);
    connect(c, &Command::finished, this, [this, c, key, gen, update](bool ok, QString out) {
        c->deleteLater();
        if (gen != generations[key])
            return;
        update(ok, out);
    });
    c->run(program, args);
}
void SystemServices::refresh() {
    query("wifi", "nmcli", {"radio", "wifi"});
    query("network", "nmcli", {"-t", "-f", "NAME,TYPE,DEVICE", "connection", "show", "--active"});
    query("networks", "nmcli",
          {"-t", "-f", "IN-USE,SSID,SIGNAL,SECURITY,BSSID", "device", "wifi", "list", "--rescan", "no"});
    query("connections", "nmcli", {"-t", "-f", "NAME,UUID,TYPE,DEVICE", "connection", "show"});
    query("volume", "wpctl", {"get-volume", "@DEFAULT_AUDIO_SINK@"});
    query("inputVolume", "wpctl", {"get-volume", "@DEFAULT_AUDIO_SOURCE@"});
    query("defaultOutput", "wpctl", {"inspect", "@DEFAULT_AUDIO_SINK@"});
    query("defaultInput", "wpctl", {"inspect", "@DEFAULT_AUDIO_SOURCE@"});
    query("audioNodes", "pw-dump", {});
    if (bluetooth)
        bluetooth->refresh();
    query("brightness", "brightnessctl", {"-c", "backlight", "-m"});
    query("power", "powerprofilesctl", {"get"});
    query("profiles", "powerprofilesctl", {"list"});
    query("displays", "kscreen-doctor", {"-j"});
    querySystem();
}
void SystemServices::execute(QString program, QStringList args, QString key) {
    if (key.isEmpty())
        key = program + ":" + args.value(0);
    for (auto& entry : pending)
        if (entry.key == key && args.value(0) != "set-mute") {
            entry = {program, args, key};
            return;
        }
    if (pending.size() >= 16) {
        status = tr("Please wait for pending changes.");
        emit changed();
        return;
    }
    pending.enqueue({program, args, key});
    startNext();
}
void SystemServices::startNext() {
    if (active || pending.isEmpty())
        return;
    auto task = pending.dequeue();
    auto c = new Command(this);
    ++active;
    status.clear();
    emit changed();
    connect(c, &Command::finished, this, [this, c, task](bool ok, QString out) {
        --active;
        status = ok ? tr("Applied; refreshing device state") : out;
        if (task.program == "bluetoothctl" && (out.contains("Failed") || out.contains("not available")))
            status = out;
        c->deleteLater();
        if (pending.isEmpty())
            refresh();
        else
            startNext();
        emit changed();
    });
    c->run(task.program, task.args, 20000);
}
void SystemServices::querySystem() {
    auto bus = QDBusConnection::sessionBus();
    auto iface = bus.interface();
    values["lockAvailable"] = iface && iface->isServiceRegistered("org.freedesktop.ScreenSaver").value();
    values["harborSession"] = iface && iface->isServiceRegistered("org.harbor.Shell").value();
    for (const auto& operation : QStringList{"Reboot", "PowerOff", "Suspend"}) {
        auto msg = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1",
                                                  "org.freedesktop.login1.Manager", "Can" + operation);
        auto watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(msg, 2000), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, operation] {
            QDBusPendingReply<QString> r = *watcher;
            values["can" + operation] = !r.isError() && (r.value() == "yes" || r.value() == "challenge");
            watcher->deleteLater();
            emit changed();
        });
    }
    auto msg = QDBusMessage::createMethodCall("org.freedesktop.UPower",
                                              "/org/freedesktop/UPower/devices/DisplayDevice",
                                              "org.freedesktop.DBus.Properties", "GetAll");
    msg << QString("org.freedesktop.UPower.Device");
    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(msg, 2000), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
        QDBusPendingReply<QVariantMap> r = *watcher;
        auto props = r.isError() ? QVariantMap{} : r.value();
        values["batteryAvailable"] = props.value("IsPresent", false);
        values["batteryPercent"] = props.value("Percentage", 0);
        values["batteryCharging"] = props.value("State").toUInt() == 1;
        watcher->deleteLater();
        emit changed();
    });
    emit changed();
}
void SystemServices::action(QString name, QVariant v) {
    if (name == "wifi")
        execute("nmcli", {"radio", "wifi", v.toBool() ? "on" : "off"});
    else if (name == "volume") {
        double n = v.toDouble();
        if (std::isfinite(n) && n >= 0 && n <= 1)
            execute("wpctl", {"set-volume", "@DEFAULT_AUDIO_SINK@", QString::number(n)}, "volume");
    } else if (name == "input-volume") {
        double n = v.toDouble();
        if (std::isfinite(n) && n >= 0 && n <= 1)
            execute("wpctl", {"set-volume", "@DEFAULT_AUDIO_SOURCE@", QString::number(n)}, "input-volume");
    } else if (name == "input-mute")
        execute("wpctl", {"set-mute", "@DEFAULT_AUDIO_SOURCE@", "toggle"}, "input-mute");
    else if (name == "audio-output" || name == "audio-input") {
        const auto rows = values[name == "audio-output" ? "audioOutputs" : "audioInputs"].toList();
        bool valid = false;
        for (auto row : rows)
            if (row.toMap()["id"].toInt() == v.toInt())
                valid = true;
        if (valid)
            execute("wpctl", {"set-default", QString::number(v.toInt())}, name);
    } else if (name == "connection-up" || name == "connection-down") {
        bool valid = false;
        for (auto row : values["savedConnections"].toList())
            if (row.toMap()["uuid"].toString() == v.toString())
                valid = true;
        if (valid)
            execute(
                "nmcli",
                {"--wait", "15", "connection", name == "connection-up" ? "up" : "down", "uuid", v.toString()},
                "connection:" + v.toString());
    } else if (name.startsWith("bluetooth")) {
        if (bluetooth)
            bluetooth->action(name, v);
    } else if (name == "reboot" || name == "poweroff" || name == "sleep") {
        const QString operation = name == "reboot" ? "Reboot" : name == "sleep" ? "Suspend" : "PowerOff";
        if (!values["can" + operation].toBool()) {
            status = tr("This session cannot request that power action.");
            emit changed();
            return;
        }
        auto msg = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1",
                                                  "org.freedesktop.login1.Manager", operation);
        msg << true;
        auto watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(msg, 30000), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
            QDBusPendingReply<> r = *watcher;
            status = r.isError() ? r.error().message() : tr("Power action requested.");
            watcher->deleteLater();
            emit changed();
        });
    } else if (name == "mute")
        execute("wpctl", {"set-mute", "@DEFAULT_AUDIO_SINK@", "toggle"});

    else if (name == "brightness") {
        int n = v.toInt();
        if (n >= 5 && n <= 100)
            execute("brightnessctl", {"-c", "backlight", "set", QString::number(n) + "%"}, "brightness");
    } else if (name == "power" &&
               QStringList{"power-saver", "balanced", "performance"}.contains(v.toString()))
        execute("powerprofilesctl", {"set", v.toString()});
    else if (name == "force-quit") {
        auto m = QDBusMessage::createMethodCall("org.kde.KWin", "/KWin", "org.kde.KWin", "killWindow");
        auto w = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(m, 5000), this);
        connect(w, &QDBusPendingCallWatcher::finished, this, [this, w] {
            QDBusPendingReply<> r = *w;
            status = r.isError() ? r.error().message() : tr("Select a window to force quit; Escape cancels.");
            w->deleteLater();
            emit changed();
        });
    } else if (name == "lock") {
        auto msg = QDBusMessage::createMethodCall("org.freedesktop.ScreenSaver", "/ScreenSaver",
                                                  "org.freedesktop.ScreenSaver", "Lock");
        auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(msg, 5000), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
            QDBusPendingReply<> r = *watcher;
            status =
                r.isError() ? tr("Lock service unavailable: ") + r.error().message() : tr("Lock requested");
            emit changed();
            watcher->deleteLater();
        });
    } else {
        status = tr("Unsupported action");
        emit changed();
    }
}
void SystemServices::openTool(QString name) {
    QMap<QString, QStringList> tools{{"network", {"nm-connection-editor"}},
                                     {"bluetooth", {"harbor-settings", "Bluetooth"}},
                                     {"audio", {"pavucontrol"}},
                                     {"users", {"user-manager"}},
                                     {"updates", {"plasma-discover"}},
                                     {"terminal", {"x-terminal-emulator"}},
                                     {"files", {"harbor-files"}},
                                     {"settings", {"harbor-settings"}}};
    if (!tools.contains(name))
        return;
    if (name == "updates") {
        for (auto candidate : {"gnome-software", "plasma-discover", "synaptic"})
            if (!QStandardPaths::findExecutable(candidate).isEmpty()) {
                tools[name] = {QString(candidate)};
                break;
            }
    }
    auto args = tools[name];
    auto cmd = args.takeFirst();
    if (!QProcess::startDetached(cmd, args)) {
        status = tr("Install the external tool: ") + cmd;
        emit changed();
    }
}

void SystemServices::applyDisplay(int output, double scale, QString mode) {
    if (!displayTransaction.isEmpty() || !std::isfinite(scale) || scale < .5 || scale > 4)
        return;
    auto runtime = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (runtime.isEmpty()) {
        status = tr("No runtime directory; cannot guard display change");
        emit changed();
        return;
    }
    QDir().mkpath(runtime + "/harbor");
    auto path = runtime + "/harbor/display-" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    QStringList args{"--directory",           path,      "--output",
                     QString::number(output), "--scale", QString::number(scale)};
    if (!mode.isEmpty())
        args << "--mode" << mode;
    if (!QProcess::startDetached("harbor-display-guard", args)) {
        status = tr("Display guard is not installed");
        emit changed();
        return;
    }
    displayTransaction = path;
    values["displayPending"] = false;
    values["displayChanging"] = true;
    displayPoll = new QTimer(this);
    auto ticks = std::make_shared<int>(0);
    connect(displayPoll, &QTimer::timeout, this, [this, ticks] {
        QFile f(displayTransaction + "/status.json");
        if (f.open(QIODevice::ReadOnly)) {
            auto o = QJsonDocument::fromJson(f.readAll()).object();
            auto state = o["status"].toString();
            values["displayPending"] = state == "pending";
            status = state == "pending" ? tr("Keep these settings? Automatic rollback after 15 seconds.")
                                        : state + " " + o["error"].toString();
            if (state != "pending") {
                displayPoll->stop();
                displayPoll->deleteLater();
                displayPoll = nullptr;
                displayTransaction.clear();
                values["displayChanging"] = false;
                refresh();
            }
        } else if (++*ticks > 80) {
            displayPoll->stop();
            displayPoll->deleteLater();
            displayPoll = nullptr;
            displayTransaction.clear();
            values["displayChanging"] = false;
            status = tr("Display guard did not report; check the display manually");
        }
        emit changed();
    });
    displayPoll->start(250);
    emit changed();
}
void SystemServices::confirmDisplay() {
    if (displayTransaction.isEmpty() || !values["displayPending"].toBool())
        return;
    QFile f(displayTransaction + "/confirm");
    if (!f.open(QIODevice::WriteOnly)) {
        status = tr("Could not confirm; display will revert");
        emit changed();
    }
}

void SystemServices::refreshRecent() {
    QFile file(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/recently-used.xbel");
    QVariantList rows;
    if (file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader xml(&file);
        QVariantMap row;
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == u"bookmark") {
                row = {{"url", xml.attributes().value("href").toString()},
                       {"modified", xml.attributes().value("modified").toString()}};
            } else if (xml.isStartElement() && xml.name() == u"title")
                row["title"] = xml.readElementText();
            else if (xml.isEndElement() && xml.name() == u"bookmark") {
                QUrl url(row["url"].toString());
                if (url.isLocalFile() && QFile::exists(url.toLocalFile())) {
                    if (row["title"].toString().isEmpty())
                        row["title"] = url.fileName();
                    rows << row;
                }
            }
        }
    }
    std::sort(rows.begin(), rows.end(), [](QVariant a, QVariant b) {
        return a.toMap()["modified"].toString() > b.toMap()["modified"].toString();
    });
    while (rows.size() > 12)
        rows.removeLast();
    values["recentItems"] = rows;
    emit changed();
}
void SystemServices::openRecent(QString url) {
    for (auto row : values["recentItems"].toList())
        if (row.toMap()["url"] == url) {
            if (!QDesktopServices::openUrl(QUrl(url))) {
                status = tr("Could not open the recent item.");
                emit changed();
            }
            return;
        }
}
