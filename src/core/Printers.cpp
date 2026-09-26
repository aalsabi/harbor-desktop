#include "Printers.h"
#include <QRegularExpression>
#include <QStandardPaths>
#include <unistd.h>
#include <QUrl>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
Printers::Printers(QObject* parent) : Printers(QDBusConnection::systemBus(), parent) {}
Printers::Printers(const QDBusConnection& connection, QObject* parent) : QObject(parent), m_bus(connection) {
    m_drivers.append(QVariantMap{{"id", "everywhere"}, {"name", tr("Driverless (IPP Everywhere)")}});
    connect(&m_command, &Command::finished, this, &Printers::completed);
}
bool Printers::validName(const QString& name) {
    if (name.isEmpty() || name.startsWith('-'))
        return false;
    for (const auto c : name)
        if (!c.isPrint() || c.isSpace() || c == '/' || c == '#')
            return false;
    return true;
}
QVariantList Printers::parsePrinters(const QString& output) {
    QVariantList rows;
    QRegularExpression re("^printer (\\S+) (.+)$");
    for (const auto& line : output.split('\n')) {
        auto match = re.match(line);
        if (match.hasMatch())
            rows.append(QVariantMap{
                {"name", match.captured(1)}, {"status", match.captured(2)}, {"jobs", QVariantList{}}});
    }
    return rows;
}
QString Printers::parseDefault(const QString& output) {
    QRegularExpression re("(?:^|\\n)system default destination: ([^\\r\\n]+)");
    return re.match(output).captured(1).trimmed();
}
QVariantList Printers::parseJobs(const QString& output) {
    QVariantList rows;
    QRegularExpression re("^(\\S+)-(\\d+)\\s+(\\S+)\\s+(\\d+)\\s*(.*)$");
    for (const auto& line : output.split('\n')) {
        auto match = re.match(line);
        if (match.hasMatch())
            rows.append(QVariantMap{{"printer", match.captured(1)},
                                    {"id", match.captured(1) + "-" + match.captured(2)},
                                    {"owner", match.captured(3)},
                                    {"bytes", match.captured(4)},
                                    {"submitted", match.captured(5)}});
    }
    return rows;
}
void Printers::refresh() {
    if (m_busy)
        return;
    m_message.clear();
    m_lpstat = QStandardPaths::findExecutable("lpstat");
    m_lpoptions = QStandardPaths::findExecutable("lpoptions");
    m_available = !m_lpstat.isEmpty();
    if (!m_available) {
        m_devices.clear();
        m_default.clear();
        m_message =
            tr("Printing tools are unavailable. Install cups-client to connect to the printing service.");
        emit changed();
        return;
    }
    m_busy = true;
    m_stage = 1;
    emit changed();
    m_command.run(m_lpstat, {"-h", "localhost", "-p", "-d"});
}
void Printers::setDefault(const QString& name) {
    if (m_busy)
        return;
    if (geteuid() == 0) {
        m_message = tr("Personal printer defaults cannot be changed while Harbor runs as root.");
        emit changed();
        return;
    }
    bool exists = false;
    for (const auto& row : m_devices)
        if (row.toMap().value("name").toString() == name)
            exists = true;
    if (!validName(name) || !exists) {
        m_message = tr("Choose a printer from the current list.");
        emit changed();
        return;
    }
    if (m_lpoptions.isEmpty()) {
        m_message = tr("Install cups-client to set your default printer.");
        emit changed();
        return;
    }
    m_busy = true;
    m_stage = 3;
    m_message.clear();
    emit changed();
    m_command.run(m_lpoptions, {"-h", "localhost", "-d", name});
}
void Printers::completed(bool ok, const QString& output) {
    if (m_stage == 7) {
        m_busy = false;
        m_discovered = ok ? parseDiscovery(output) : QVariantList{};
        if (!ok)
            m_message = output;
        else if (m_discovered.isEmpty())
            m_message = tr("No printers found. Check power and network connection, then search again.");
        emit changed();
        return;
    }
    if (m_stage == 4) {
        m_busy = false;
        if (ok) {
            m_drivers = parseDrivers(output);
            if (m_device.isEmpty() || m_device.value("driverless").toBool())
                m_drivers.prepend(
                    QVariantMap{{"id", "everywhere"}, {"name", tr("Driverless (IPP Everywhere)")}});
        } else
            m_message = output;
        emit changed();
        return;
    }
    if (m_stage == 5) {
        m_busy = false;
        m_options = ok ? parseOptions(output) : QVariantList{};
        if (!ok)
            m_message = output;
        emit changed();
        return;
    }
    if (m_stage == 6) {
        m_busy = false;
        if (ok)
            loadOptions(m_selected);
        else {
            m_message = output;
            emit changed();
        }
        return;
    }

    if (m_stage == 1) {
        if (!ok) {
            m_devices.clear();
            m_default.clear();
            m_message = output.isEmpty() ? tr("The printing service is unavailable.") : output;
            m_busy = false;
            emit changed();
            return;
        }
        m_devices = parsePrinters(output);
        m_default = parseDefault(output);
        m_stage = 2;
        m_command.run(m_lpstat, {"-h", "localhost", "-o"});
        return;
    }
    if (m_stage == 2) {
        if (ok) {
            auto jobs = parseJobs(output);
            for (auto& entry : m_devices) {
                auto row = entry.toMap();
                QVariantList queue;
                for (const auto& job : jobs)
                    if (job.toMap().value("printer") == row.value("name"))
                        queue.append(job);
                row.insert("jobs", queue);
                entry = row;
            }
        } else
            m_message = tr("Printers loaded, but the queue could not be read: %1").arg(output);
        m_busy = false;
        emit changed();
        return;
    }
    m_busy = false;
    if (!ok) {
        m_message = output;
        emit changed();
        return;
    }
    refresh();
}

bool Printers::hasPrinter(const QString& name) const {
    for (const auto& entry : m_devices)
        if (entry.toMap().value("name").toString().compare(name, Qt::CaseInsensitive) == 0)
            return true;
    return false;
}
bool Printers::validUri(const QString& uri) {
    QUrl url(uri, QUrl::StrictMode);
    return uri.size() <= 2048 && url.isValid() && (url.scheme() == "ipp" || url.scheme() == "ipps") &&
           !url.host().isEmpty() && url.userInfo().isEmpty() && !url.hasFragment() && !url.hasQuery() &&
           !uri.contains(QRegularExpression("[\\s\\x00-\\x1f]"));
}
QVariantList Printers::parseDrivers(const QString& output) {
    QVariantList rows;
    QRegularExpression re("^(\\S+) (.+)$");
    for (const auto& line : output.split('\n')) {
        auto m = re.match(line);
        if (m.hasMatch() && m.captured(1) != "everywhere")
            rows.append(QVariantMap{{"id", m.captured(1)}, {"name", m.captured(2)}});
    }
    return rows;
}
QVariantList Printers::parseOptions(const QString& output) {
    QVariantList rows;
    QRegularExpression re("^([A-Za-z0-9_.-]+)/([^:]+): (.+)$");
    for (const auto& line : output.split('\n')) {
        auto m = re.match(line);
        if (!m.hasMatch())
            continue;
        QVariantList choices;
        int current = -1;
        for (auto value : m.captured(3).split(' ', Qt::SkipEmptyParts)) {
            if (value.startsWith('*')) {
                value.remove(0, 1);
                current = choices.size();
            }
            if (!QRegularExpression("^[A-Za-z0-9_.-]+$").match(value).hasMatch())
                continue;
            choices.append(value);
        }
        if (!choices.isEmpty())
            rows.append(QVariantMap{{"key", m.captured(1)},
                                    {"name", m.captured(2)},
                                    {"choices", choices},
                                    {"currentIndex", current}});
    }
    return rows;
}
void Printers::loadDrivers() {
    if (m_busy)
        return;
    auto program = QStandardPaths::findExecutable("lpinfo");
    if (program.isEmpty())
        program = QStandardPaths::findExecutable("lpinfo", {"/usr/sbin", "/usr/bin"});
    if (program.isEmpty()) {
        m_message = tr("Install cups-client to list installed drivers.");
        emit changed();
        return;
    }
    m_busy = true;
    m_message.clear();
    m_stage = 4;
    emit changed();
    QStringList args{"-h", "localhost"};
    if (!m_device.value("deviceId").toString().isEmpty())
        args << "--device-id" << m_device.value("deviceId").toString();
    args << "-m";
    m_command.run(program, args, 15000);
}
void Printers::loadOptions(const QString& name) {
    if (m_busy)
        return;
    if (!hasPrinter(name) || !validName(name)) {
        m_message = tr("Choose a printer from the current list.");
        emit changed();
        return;
    }
    if (m_lpoptions.isEmpty()) {
        m_message = tr("Install cups-client to read printing options.");
        emit changed();
        return;
    }
    m_selected = name;
    m_options.clear();
    m_busy = true;
    m_message.clear();
    m_stage = 5;
    emit changed();
    m_command.run(m_lpoptions, {"-h", "localhost", "-p", name, "-l"});
}
void Printers::setOption(const QString& name, const QString& key, const QString& value) {
    if (m_busy)
        return;
    if (geteuid() == 0) {
        m_message = tr("Personal printing options cannot be changed as root.");
        emit changed();
        return;
    }
    bool valid = false;
    for (const auto& entry : m_options) {
        auto row = entry.toMap();
        if (row.value("key").toString() == key && row.value("choices").toList().contains(value))
            valid = true;
    }
    if (name != m_selected || !hasPrinter(name) || !valid) {
        m_message = tr("Choose a currently available printing option.");
        emit changed();
        return;
    }
    m_busy = true;
    m_message.clear();
    m_stage = 6;
    emit changed();
    m_command.run(m_lpoptions, {"-h", "localhost", "-p", name, "-o", key + "=" + value});
}
void Printers::adminCall(const QString& method, const QVariantList& args, std::function<void()> after) {
    m_busy = true;
    m_message.clear();
    emit changed();
    auto msg = QDBusMessage::createMethodCall("org.opensuse.CupsPkHelper.Mechanism", "/",
                                              "org.opensuse.CupsPkHelper.Mechanism", method);
    msg.setArguments(args);
    msg.setInteractiveAuthorizationAllowed(true);
    auto w = new QDBusPendingCallWatcher(m_bus.asyncCall(msg, 120000), this);
    connect(w, &QDBusPendingCallWatcher::finished, this, [this, w, after] {
        QDBusPendingReply<QString> reply = *w;
        w->deleteLater();
        if (reply.isError() || !reply.value().isEmpty()) {
            m_busy = false;
            m_message = reply.isError()
                            ? tr("Printer administration failed. Ensure cups-pk-helper is installed: %1")
                                  .arg(reply.error().message())
                            : reply.value();
            emit changed();
            return;
        }
        if (after) {
            after();
            return;
        }
        m_busy = false;
        refresh();
    });
}
void Printers::addPrinter(const QString& name, const QString& uri, const QString& driver) {
    if (m_busy)
        return;
    bool validDriver = false;
    for (const auto& entry : m_drivers)
        if (entry.toMap().value("id").toString() == driver)
            validDriver = true;
    bool knownDevice = false;
    for (const auto& entry : m_discovered)
        if (entry.toMap().value("uri").toString() == uri)
            knownDevice = true;
    if (!validName(name) || name.toUtf8().size() > 127 || hasPrinter(name) ||
        !(validUri(uri) || (knownDevice && validDeviceUri(uri))) || !validDriver ||
        (driver == "everywhere" && !validUri(uri) && !uri.contains("._ipp._tcp") &&
         !uri.contains("._ipps._tcp"))) {
        m_message = tr(
            "Use a new printer name, an ipp:// or ipps:// address without credentials, and an available driver.");
        emit changed();
        return;
    }
    adminCall("PrinterAdd", {name, uri, driver, name, QString()}, [this, name] {
        adminCall("PrinterSetEnabled", {name, true}, [this, name] {
            adminCall("PrinterSetAcceptJobs", {name, true, QString()});
        });
    });
}
void Printers::removePrinter(const QString& name, bool confirmed) {
    if (m_busy)
        return;
    if (!confirmed || !hasPrinter(name)) {
        m_message = tr("Confirm removal of a printer from the current list.");
        emit changed();
        return;
    }
    adminCall("PrinterDelete", {name});
}
void Printers::cancelJob(const QString& id, bool confirmed) {
    if (m_busy)
        return;
    bool found = false;
    for (const auto& entry : m_devices)
        for (const auto& job : entry.toMap().value("jobs").toList())
            if (job.toMap().value("id").toString() == id)
                found = true;
    bool ok = false;
    int number = id.mid(id.lastIndexOf('-') + 1).toInt(&ok);
    if (!confirmed || !found || !ok || number <= 0) {
        m_message = tr("Confirm cancellation of a job from the current queue.");
        emit changed();
        return;
    }
    adminCall("JobCancelPurge", {number, false});
}

bool Printers::validDeviceUri(const QString& uri) {
    if (validUri(uri))
        return true;
    QUrl u(uri, QUrl::StrictMode);
    return uri.size() <= 2048 && u.isValid() && !u.host().isEmpty() && u.userInfo().isEmpty() &&
           !u.hasFragment() && !uri.contains(QRegularExpression("[\\s\\x00-\\x1f]")) &&
           QStringList{"usb", "dnssd", "socket", "lpd"}.contains(u.scheme());
}
QVariantList Printers::parseDiscovery(const QString& output) {
    QVariantList rows;
    QVariantMap row;
    QStringList seen;
    auto append = [&] {
        auto uri = row.value("uri").toString();
        if (!validDeviceUri(uri) || seen.contains(uri))
            return;
        seen.append(uri);
        if (row.value("name").toString().isEmpty())
            row["name"] = uri;
        row["driverless"] = validUri(uri) || uri.contains("._ipp._tcp") || uri.contains("._ipps._tcp");
        rows.append(row);
    };
    for (const auto& line : output.split('\n')) {
        if (line.startsWith("Device: ")) {
            append();
            row.clear();
            row["uri"] = line.mid(8).trimmed();
        } else {
            auto t = line.trimmed();
            if (t.startsWith("info = "))
                row["name"] = t.mid(7);
            else if (t.startsWith("make-and-model = "))
                row["model"] = t.mid(17);
            else if (t.startsWith("device-id = "))
                row["deviceId"] = t.mid(12);
        }
    }
    append();
    return rows;
}
void Printers::discover() {
    if (m_busy)
        return;
    m_device.clear();
    m_discovered.clear();
    m_message.clear();
    auto program = QStandardPaths::findExecutable("lpinfo");
    if (program.isEmpty())
        program = QStandardPaths::findExecutable("lpinfo", {"/usr/sbin", "/usr/bin"});
    if (program.isEmpty()) {
        m_message = tr("Install cups-client and start the CUPS printing service to discover printers.");
        emit changed();
        return;
    }
    m_busy = true;
    m_stage = 7;
    emit changed();
    m_command.run(program, {"-h", "localhost", "-l", "--timeout", "8", "-v"}, 15000);
}
void Printers::selectDevice(const QString& uri) {
    if (m_busy)
        return;
    for (const auto& entry : m_discovered) {
        auto row = entry.toMap();
        if (row.value("uri").toString() == uri) {
            m_device = row;
            m_drivers.clear();
            if (row.value("driverless").toBool())
                m_drivers.append(
                    QVariantMap{{"id", "everywhere"}, {"name", tr("Driverless (IPP Everywhere)")}});
            emit changed();
            loadDrivers();
            return;
        }
    }
    m_message = tr("Choose a printer from the latest discovery results.");
    emit changed();
}
