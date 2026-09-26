#include "SystemState.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
static QStringList fields(QString line) {
    QStringList result;
    QString current;
    bool escaped = false;
    for (auto ch : line) {
        if (escaped) {
            current += ch;
            escaped = false;
        } else if (ch == '\\')
            escaped = true;
        else if (ch == ':') {
            result << current;
            current.clear();
        } else
            current += ch;
    }
    if (escaped)
        current += '\\';
    result << current;
    return result;
}
QVariantMap SystemState::parse(const QString& key, const QString& output) {
    QVariantMap result;
    if (key == "volume" || key == "inputVolume") {
        const auto match = QRegularExpression("Volume:\\s*([0-9.]+)").match(output);
        const QString prefix = key == "volume" ? "output" : "input";
        result[prefix + "Volume"] = match.hasMatch() ? match.captured(1).toDouble() : 0.;
        result[prefix + "Muted"] = output.contains("[MUTED]");
    } else if (key == "brightness") {
        auto parts = output.section('\n', 0, 0).split(',');
        QString number = parts.value(3);
        number.remove('%');
        bool valid = false;
        int value = number.toInt(&valid);
        result["brightnessPercent"] = valid ? qBound(0, value, 100) : 0;
    } else if (key == "networks") {
        QVariantList rows;
        for (auto line : output.split('\n', Qt::SkipEmptyParts)) {
            auto f = fields(line);
            if (f.size() < 5 || f[1].isEmpty())
                continue;
            rows.append(QVariantMap{{"name", f[1]},
                                    {"active", f[0] == "*"},
                                    {"signal", f[2].toInt()},
                                    {"security", f[3]},
                                    {"address", f[4]}});
        }
        result["wifiNetworks"] = rows;
    } else if (key == "connections") {
        QVariantList rows;
        for (auto line : output.split('\n', Qt::SkipEmptyParts)) {
            auto f = fields(line);
            if (f.size() < 4)
                continue;
            rows.append(QVariantMap{
                {"name", f[0]}, {"uuid", f[1]}, {"type", f[2]}, {"active", !f[3].isEmpty() && f[3] != "--"}});
        }
        result["savedConnections"] = rows;
    } else if (key == "devices" || key == "connectedDevices") {
        QVariantList rows;
        QStringList addresses;
        QRegularExpression expression("^Device ([0-9A-Fa-f:]{17}) (.+)$");
        for (auto line : output.split('\n', Qt::SkipEmptyParts)) {
            auto match = expression.match(line);
            if (!match.hasMatch())
                continue;
            addresses << match.captured(1).toUpper();
            rows.append(QVariantMap{{"address", match.captured(1).toUpper()}, {"name", match.captured(2)}});
        }
        result[key == "devices" ? "bluetoothDevices" : "bluetoothConnectedRows"] = rows;
        if (key == "connectedDevices")
            result["bluetoothConnected"] = addresses;
    } else if (key == "profiles") {
        QStringList profiles;
        for (auto line : output.split('\n')) {
            auto name = line.trimmed();
            if (name.startsWith('*'))
                name = name.mid(1).trimmed();
            if (name.endsWith(':'))
                name.chop(1);
            if (QStringList{"power-saver", "balanced", "performance"}.contains(name))
                profiles << name;
        }
        result["powerProfiles"] = profiles;
    } else if (key == "audioNodes") {
        QVariantList outputs, inputs;
        for (auto value : QJsonDocument::fromJson(output.toUtf8()).array()) {
            auto obj = value.toObject();
            if (obj["type"] != "PipeWire:Interface:Node")
                continue;
            auto props = obj["info"].toObject()["props"].toObject();
            auto type = props["media.class"].toString();
            if (type != "Audio/Sink" && type != "Audio/Source")
                continue;
            QVariantMap row{{"id", obj["id"].toInt()},
                            {"name", props["node.description"].toString(props["node.name"].toString())}};
            (type == "Audio/Sink" ? outputs : inputs).append(row);
        }
        result["audioOutputs"] = outputs;
        result["audioInputs"] = inputs;
    } else if (key == "defaultOutput" || key == "defaultInput") {
        auto match = QRegularExpression("^id (\\d+),").match(output);
        result[key + "Id"] = match.hasMatch() ? match.captured(1).toInt() : -1;
    }
    return result;
}
