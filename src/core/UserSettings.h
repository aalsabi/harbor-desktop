#pragma once
#include <QObject>
#include <QQueue>
#include <QVariantMap>
#include "Command.h"
class UserSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap state READ state NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
public:
    explicit UserSettings(QObject* parent = nullptr);
    QVariantMap state() const { return values; }
    bool busy() const { return command.busy(); }
    QString error() const { return problem; }
    Q_INVOKABLE void request(QString operation, QVariantMap data = {});
signals:
    void changed();

private:
    void next();
    QQueue<QPair<QString, QVariantMap>> queued;
    Command command;
    QVariantMap values;
    QString problem, current;
};
