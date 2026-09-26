#pragma once
#include <QObject>
#include <QVariantList>
#include <QTimer>
#include <QElapsedTimer>
#include "Command.h"
class DisplaySettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList outputs READ outputs NOTIFY changed)
    Q_PROPERTY(QVariantList monitors READ monitors NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool pending READ pending NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QString brightnessError READ brightnessError NOTIFY changed)
public:
    explicit DisplaySettings(QObject* parent = nullptr, int transactionTimeoutMs = 80000);
    QVariantList outputs() const { return m_outputs; }
    QVariantList monitors() const { return m_monitors; }
    bool busy() const { return m_busy; }
    bool pending() const { return m_pending; }
    QString error() const { return m_error; }
    QString brightnessError() const { return m_brightnessError; }
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void detectBrightness();
    Q_INVOKABLE void setBrightness(int bus, int percent);
    Q_INVOKABLE void apply(int id, int x, int y, double scale, QString mode, bool primary);
    Q_INVOKABLE void confirm();
    Q_INVOKABLE void revert();
    static QVariantList parseMonitors(const QString& out);
    static QVariantMap parseVcp(const QString& out);
signals:
    void changed();

private:
    void complete(bool ok, QString out);
    void nextMonitor();
    void poll();
    void signalTransaction(const QString& name);
    Command m_command;
    QTimer m_poll;
    QElapsedTimer m_transactionTimer;
    int m_transactionTimeoutMs = 80000;
    QVariantList m_outputs, m_monitors, m_candidates;
    QString m_error, m_brightnessError, m_transaction;
    int m_stage = 0, m_index = 0;
    bool m_busy = false, m_pending = false;
};
