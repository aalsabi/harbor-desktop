#pragma once
#include <QObject>
#include <QSettings>
#include <QFileSystemWatcher>
#include <QVariantList>
#include <QDateTime>
#include <QTimer>
class NotificationPreferences : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList profiles READ profiles NOTIFY changed)
    Q_PROPERTY(QString focusMode READ focusMode WRITE setFocusMode NOTIFY changed)
    Q_PROPERTY(QVariantMap activeProfile READ activeProfile NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(bool doNotDisturb READ doNotDisturb WRITE setDoNotDisturb NOTIFY changed)
    Q_PROPERTY(QVariantList applications READ applications NOTIFY changed)
public:
    explicit NotificationPreferences(QString path = {}, QObject* parent = nullptr);
    QVariantList profiles() const;
    QString focusMode() const;
    QVariantMap activeProfile() const;
    QString error() const { return problem; }
    bool attentionAllowed(QString id) const;
    static bool scheduleMatches(const QVariantMap& profile, const QDateTime& now);
    Q_INVOKABLE bool saveProfile(QVariantMap profile);
    Q_INVOKABLE void removeProfile(QString id);
    Q_INVOKABLE void setFocusMode(QString mode);
    bool doNotDisturb() const;
    QVariantList applications() const;
    bool enabled(QString id) const;
    bool preview(QString id) const;
    void recordApplication(QString id, QString name);
    Q_INVOKABLE void setDoNotDisturb(bool value);
    Q_INVOKABLE void setEnabled(QString id, bool value);
    Q_INVOKABLE void setPreview(QString id, bool value);
signals:
    void changed();

private:
    QString group(QString id) const;
    void set(QString id, QString key, bool value);
    QSettings settings;
    QFileSystemWatcher watcher;
    QTimer timer;
    QString problem, lastActive;
};
