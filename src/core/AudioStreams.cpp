#include "AudioStreams.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <cmath>
AudioStreams::AudioStreams(QObject* parent) : QObject(parent) {
    connect(&m_command, &Command::finished, this, &AudioStreams::complete);
    m_timer.setInterval(4000);
    connect(&m_timer, &QTimer::timeout, this, &AudioStreams::refresh);
}
QVariantList AudioStreams::parse(const QByteArray& json, bool streams, bool* valid) {
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(json, &error);
    bool ok = error.error == QJsonParseError::NoError && doc.isArray();
    if (valid)
        *valid = ok;
    QVariantList list;
    if (!ok)
        return list;
    for (const auto& entry : doc.array()) {
        auto o = entry.toObject();
        if (!o["index"].isDouble() || o["index"].toInt(-1) < 0)
            continue;
        auto props = o["properties"].toObject();
        QString name = streams ? props["application.name"].toString() : o["description"].toString();
        if (name.isEmpty())
            name = o["name"].toString(props["media.name"].toString());
        if (name.isEmpty())
            name = QString::number(o["index"].toInt());
        QVariantMap row{{"id", o["index"].toInt()},
                        {"name", name},
                        {"description", props["media.name"].toString()},
                        {"sink", o["sink"].toInt(-1)},
                        {"muted", o["mute"].toBool()}};
        auto volumes = o["volume"].toObject();
        double volume = 0;
        int count = 0;
        for (auto v : volumes) {
            auto channel = v.toObject();
            if (channel["value"].isDouble()) {
                volume += channel["value"].toDouble() / 65536.0;
                ++count;
            }
        }
        row["volume"] = count ? qRound(volume * 100 / count) : 0;
        row["canVolume"] = count > 0 && o["has_volume"].toBool(true) && o["volume_writable"].toBool(true);
        list << row;
    }
    return list;
}
void AudioStreams::setActive(bool active) {
    if (active) {
        refresh();
        m_timer.start();
    } else
        m_timer.stop();
}
void AudioStreams::refresh() {
    if (m_busy)
        return;
    m_busy = true;
    m_stage = 1;
    emit changed();
    m_command.run("pactl", {"-f", "json", "list", "sinks"});
}
void AudioStreams::complete(bool ok, const QString& out) {
    if (m_stage == 3) {
        m_busy = false;
        if (!ok) {
            m_error = out;
            emit streamsChanged();
            emit changed();
            return;
        }
        m_error.clear();
        refresh();
        return;
    }
    bool valid = false;
    auto rows = ok ? parse(out.toUtf8(), m_stage == 2, &valid) : QVariantList{};
    if (!ok || !valid) {
        m_available = false;
        m_error = out.isEmpty() ? tr("The audio service did not return a valid device list.") : out;
        m_streams.clear();
        m_sinks.clear();
        emit streamsChanged();
        emit sinksChanged();
        m_busy = false;
        emit changed();
        return;
    }
    if (m_stage == 1) {
        if (m_sinks != rows) {
            m_sinks = rows;
            emit sinksChanged();
        }
        m_stage = 2;
        m_command.run("pactl", {"-f", "json", "list", "sink-inputs"});
        return;
    }
    if (m_streams != rows) {
        m_streams = rows;
        emit streamsChanged();
    }
    m_error.clear();
    m_available = true;
    m_busy = false;
    emit changed();
}
void AudioStreams::mutate(int id, const QStringList& args) {
    if (m_busy)
        return;
    bool found = false;
    for (auto item : m_streams)
        if (item.toMap()["id"].toInt() == id)
            found = true;
    if (!found) {
        m_error = tr("This application is no longer playing audio. Refresh the list.");
        emit changed();
        return;
    }
    m_busy = true;
    m_stage = 3;
    m_error.clear();
    emit changed();
    m_command.run("pactl", args);
}
void AudioStreams::setVolume(int id, int percent) {
    if (percent < 0 || percent > 100)
        return;
    for (auto row : m_streams)
        if (row.toMap()["id"].toInt() == id && !row.toMap()["canVolume"].toBool())
            return;
    mutate(id, {"set-sink-input-volume", QString::number(id), QString::number(percent) + "%"});
}
void AudioStreams::setMuted(int id, bool muted) {
    mutate(id, {"set-sink-input-mute", QString::number(id), muted ? "1" : "0"});
}
void AudioStreams::move(int id, int sink) {
    bool found = false;
    for (auto row : m_sinks)
        if (row.toMap()["id"].toInt() == sink)
            found = true;
    if (!found) {
        m_error = tr("Choose an output from the current device list.");
        emit changed();
        return;
    }
    mutate(id, {"move-sink-input", QString::number(id), QString::number(sink)});
}
