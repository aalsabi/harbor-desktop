#pragma once
#include <QObject>
#include <QProcess>
#include <QTimer>
class Command : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
public:
    explicit Command(QObject* parent = nullptr);
    bool busy() const { return active; }
    // Starts `program` unless a previous run is still active. Returns false and logs a warning when
    // busy; `finished` is then emitted only for the command already running.
    bool run(const QString& program, const QStringList& args, int timeout = 5000);
signals:
    void finished(bool ok, QString output);
    void busyChanged();

private:
    void finish(bool ok, QString text);
    QProcess process;
    QTimer timer;
    bool active = false;
    bool timedOut = false;
};
