#pragma once
#include <QObject>
#include <QVariantList>
#include <QTimer>
#include "Command.h"
class AudioStreams : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList streams READ streams NOTIFY streamsChanged)
    Q_PROPERTY(QVariantList sinks READ sinks NOTIFY sinksChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
public:
    explicit AudioStreams(QObject* parent = nullptr);
    QVariantList streams() const { return m_streams; }
    QVariantList sinks() const { return m_sinks; }
    bool busy() const { return m_busy; }
    bool available() const { return m_available; }
    QString error() const { return m_error; }
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setActive(bool active);
    Q_INVOKABLE void setVolume(int id, int percent);
    Q_INVOKABLE void setMuted(int id, bool muted);
    Q_INVOKABLE void move(int id, int sink);
    static QVariantList parse(const QByteArray& json, bool streams, bool* valid = nullptr);
signals:
    void changed();
    void streamsChanged();
    void sinksChanged();

private:
    void mutate(int id, const QStringList& args);
    void complete(bool ok, const QString& out);
    Command m_command;
    QTimer m_timer;
    QVariantList m_streams, m_sinks;
    QString m_error;
    int m_stage = 0;
    bool m_busy = false, m_available = false;
};
