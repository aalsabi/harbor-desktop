#pragma once
#include <QObject>
#include <QSettings>
#include <QFileSystemWatcher>
class Preferences : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString accent READ accent WRITE setAccent NOTIFY changed)
    Q_PROPERTY(QString wallpaper READ wallpaper WRITE setWallpaper NOTIFY changed)
    Q_PROPERTY(int dockIconSize READ dockIconSize WRITE setDockIconSize NOTIFY changed)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY changed)
    Q_PROPERTY(double opacity READ opacity WRITE setOpacity NOTIFY changed)
    Q_PROPERTY(bool dark READ dark WRITE setDark NOTIFY changed)
    Q_PROPERTY(bool reduceMotion READ reduceMotion WRITE setReduceMotion NOTIFY changed)
    Q_PROPERTY(QStringList pins READ pins WRITE setPins NOTIFY changed)
public:
    explicit Preferences(QString path = {}, QObject* parent = nullptr);
    QString accent() const;
    void setAccent(QString);
    QString wallpaper() const;
    void setWallpaper(QString);
    int dockIconSize() const;
    void setDockIconSize(int);
    QString language() const;
    void setLanguage(QString value);
    double opacity() const;
    bool dark() const;
    bool reduceMotion() const;
    QStringList pins() const;
    void setOpacity(double v);
    void setDark(bool v);
    void setReduceMotion(bool v);
    void setPins(QStringList v);
signals:
    void changed();

private:
    QSettings settings;
    QFileSystemWatcher watcher;
};
