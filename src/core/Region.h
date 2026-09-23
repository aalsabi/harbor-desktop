#pragma once
#include <QObject>
#include <QVariantMap>
#include <QFileSystemWatcher>
class Region : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap state READ state NOTIFY changed)
    Q_PROPERTY(QVariantList languages READ languages CONSTANT)
    Q_PROPERTY(QVariantList regions READ regions CONSTANT)
    Q_PROPERTY(QVariantList currencies READ currencies CONSTANT)
    Q_PROPERTY(QVariantList calendars READ calendars CONSTANT)
    Q_PROPERTY(QString localeNotice READ localeNotice NOTIFY changed)
    Q_PROPERTY(QString message READ message NOTIFY changed)
public:
    explicit Region(QString configRoot = {}, QObject *parent = nullptr);
    QVariantMap state() const { return m_state; }
    QVariantList languages() const { return m_languages; }
    QVariantList regions() const { return m_regions; }
    QVariantList currencies() const { return m_currencies; }
    QVariantList calendars() const { return m_calendars; }
    QString localeNotice() const;
    static QString availabilityNotice(const QVariantMap &state, const QStringList &availableLocales);
    QString message() const { return m_message; }
    Q_INVOKABLE QVariantMap defaults(QString region) const;
    Q_INVOKABLE QVariantMap preview(QVariantMap draft) const;
    Q_INVOKABLE bool apply(QVariantMap draft);
    Q_INVOKABLE QString clockText() const;
signals:
    void changed();
private:
    QString validate(const QVariantMap &draft) const;
    void reload();
    void watch();
    QString m_path, m_message;
    QStringList m_availableLocales;
    QVariantMap m_state;
    QVariantList m_languages, m_regions, m_calendars, m_currencies;
    QFileSystemWatcher m_watcher;
};
