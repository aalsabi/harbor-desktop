#include "Preferences.h"
#include <QStandardPaths>
#include <cmath>
#include <QDir>
#include <QFileInfo>
Preferences::Preferences(QString path, QObject* parent)
    : QObject(parent),
      settings(path.isEmpty()
                   ? QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/harbor/settings.ini"
                   : path,
               QSettings::IniFormat) {
    auto directory = QFileInfo(settings.fileName()).absolutePath();
    QDir().mkpath(directory);
    watcher.addPath(directory);
    connect(&watcher, &QFileSystemWatcher::directoryChanged, this, [this] {
        settings.sync();
        emit changed();
    });
}
double Preferences::opacity() const {
    auto v = settings.value("opacity", .9).toDouble();
    return std::isfinite(v) && v >= .45 && v <= 1 ? v : .9;
}
bool Preferences::dark() const {
    return settings.value("dark", false).toBool();
}
bool Preferences::reduceMotion() const {
    return settings.value("reduceMotion", false).toBool();
}
QStringList Preferences::pins() const {
    return settings
        .value("pins", QStringList{"org.kde.dolphin.desktop", "firefox-esr.desktop",
                                   "org.kde.konsole.desktop", "org.harbor.Settings.desktop"})
        .toStringList();
}
void Preferences::setOpacity(double v) {
    if (!std::isfinite(v) || v < .45 || v > 1)
        return;
    settings.setValue("opacity", v);
    settings.sync();
    emit changed();
}
void Preferences::setDark(bool v) {
    settings.setValue("dark", v);
    settings.sync();
    emit changed();
}
void Preferences::setReduceMotion(bool v) {
    settings.setValue("reduceMotion", v);
    settings.sync();
    emit changed();
}
void Preferences::setPins(QStringList v) {
    v.removeDuplicates();
    settings.setValue("pins", v);
    settings.sync();
    emit changed();
}

QString Preferences::language() const {
    return settings.value("language", "en").toString();
}
void Preferences::setLanguage(QString v) {
    if (v != "en" && v != "ar")
        return;
    settings.setValue("language", v);
    settings.sync();
    emit changed();
}

static const QStringList accents{"#1684f8", "#168044", "#7955c9", "#b65b00", "#c63f75"};
QString Preferences::accent() const {
    auto v = settings.value("accent", "#1684f8").toString();
    return accents.contains(v) ? v : accents[0];
}
void Preferences::setAccent(QString v) {
    if (!accents.contains(v))
        return;
    settings.setValue("accent", v);
    settings.sync();
    emit changed();
}
QString Preferences::wallpaper() const {
    auto v = settings.value("wallpaper", "harbor").toString();
    return QStringList{"harbor", "sunset", "forest"}.contains(v) ? v : QString("harbor");
}
void Preferences::setWallpaper(QString v) {
    if (!QStringList{"harbor", "sunset", "forest"}.contains(v))
        return;
    settings.setValue("wallpaper", v);
    settings.sync();
    emit changed();
}
int Preferences::dockIconSize() const {
    auto v = settings.value("dockIconSize", 48).toInt();
    return v >= 32 && v <= 56 ? v : 48;
}
void Preferences::setDockIconSize(int v) {
    if (v < 32 || v > 56)
        return;
    settings.setValue("dockIconSize", v);
    settings.sync();
    emit changed();
}
