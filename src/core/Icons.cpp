#include "Icons.h"
#include <QIcon>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>

void Icons::configureTheme() {
    // A custom Wayland shell may have no desktop platform-theme plugin. Qt then
    // exposes only :/icons; add the freedesktop data locations explicitly.
    auto search = QIcon::themeSearchPaths();
    auto fallback = QIcon::fallbackSearchPaths();
    search.prepend(QDir::homePath() + "/.icons");
    const auto data = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    QStringList installed;
    for (const auto& path : data) {
        installed << QDir::cleanPath(path + "/icons");
        fallback << QDir::cleanPath(path + "/pixmaps");
    }
    search = installed + search;
    search.removeDuplicates();
    fallback.removeDuplicates();
    QIcon::setThemeSearchPaths(search);
    QIcon::setFallbackSearchPaths(fallback);
    const auto harborFile =
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/harbor/settings.ini";
    QSettings harbor(harborFile, QSettings::IniFormat);
    const auto selected = harbor.value("iconTheme").toString().trimmed();
    // An explicit Harbor choice takes priority over KDE and platform defaults.
    // Harbor uses the KDE settings even when no Plasma platform theme is loaded.
    const auto config = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, "kdeglobals");
    if (!selected.isEmpty())
        QIcon::setThemeName(selected);
    else if (!config.isEmpty()) {
        QSettings settings(config, QSettings::IniFormat);
        const auto theme = settings.value("Icons/Theme").toString().trimmed();
        if (!theme.isEmpty())
            QIcon::setThemeName(theme);
    }
    if (QIcon::themeName().isEmpty())
        QIcon::setThemeName("breeze");
    if (QIcon::fallbackThemeName().isEmpty())
        QIcon::setFallbackThemeName("hicolor");
}
QPixmap Icons::requestPixmap(const QString& id, QSize* size, const QSize& requested) {
    const QSize target = requested.isValid() ? requested : QSize(64, 64);
    const QString name = QFile::exists(id) ? id : QUrl::fromPercentEncoding(id.toUtf8());
    QIcon icon = QDir::isAbsolutePath(name) ? QIcon(name) : QIcon::fromTheme(name);
    if (icon.isNull()) {
        const QString bundled = ":/assets/icons/" + name + ".svg";
        icon = QIcon(QFile::exists(bundled) ? bundled : QStringLiteral(":/assets/icons/app.svg"));
    }
    auto pixmap = icon.pixmap(target);
    if (size)
        *size = pixmap.size();
    return pixmap;
}
