#include "DefaultApps.h"
#include <QMap>
#include <QSet>
#include <algorithm>
#undef signals
#include <gio/gdesktopappinfo.h>
namespace {
const QMap<QString, QStringList> types{{"browser", {"x-scheme-handler/http", "x-scheme-handler/https"}},
                                       {"mail", {"x-scheme-handler/mailto"}},
                                       {"files", {"inode/directory"}},
                                       {"pdf", {"application/pdf"}},
                                       {"text", {"text/plain"}},
                                       {"images", {"image/png", "image/jpeg"}}};
QVariantList candidates(const QStringList& mimeTypes) {
    QMap<QString, QVariantMap> entries;
    QSet<QString> common;
    bool first = true;
    for (const auto& mime : mimeTypes) {
        QSet<QString> found;
        auto apps = g_app_info_get_all_for_type(mime.toUtf8().constData());
        for (auto l = apps; l; l = l->next) {
            auto app = G_APP_INFO(l->data);
            auto raw = g_app_info_get_id(app);
            if (!raw || !g_app_info_should_show(app))
                continue;
            QString id = QString::fromUtf8(raw);
            found.insert(id);
            QString icon;
            auto gi = g_app_info_get_icon(app);
            if (gi && G_IS_THEMED_ICON(gi)) {
                auto names = g_themed_icon_get_names(G_THEMED_ICON(gi));
                if (names && names[0])
                    icon = QString::fromUtf8(names[0]);
            }
            entries.insert(
                id,
                {{"id", id}, {"name", QString::fromUtf8(g_app_info_get_display_name(app))}, {"icon", icon}});
        }
        g_list_free_full(apps, g_object_unref);
        if (first) {
            common = found;
            first = false;
        } else
            common.intersect(found);
    }
    QVariantList result;
    for (const auto& id : common)
        result.append(entries.value(id));
    std::sort(result.begin(), result.end(), [](const QVariant& a, const QVariant& b) {
        return QString::localeAwareCompare(a.toMap().value("name").toString(),
                                           b.toMap().value("name").toString()) < 0;
    });
    return result;
}
} // namespace
DefaultApps::DefaultApps(QObject* parent) : QObject(parent) {}
void DefaultApps::refresh() {
    m_roles.clear();
    for (const auto& key : QStringList{"browser", "mail", "files", "pdf", "text", "images"}) {
        auto mimeTypes = types.value(key);
        QString currentId, currentName;
        bool mixed = false;
        bool first = true;
        for (const auto& mime : mimeTypes) {
            auto app = g_app_info_get_default_for_type(mime.toUtf8().constData(), false);
            QString id = app ? QString::fromUtf8(g_app_info_get_id(app)) : QString();
            if (first) {
                currentId = id;
                currentName = app ? QString::fromUtf8(g_app_info_get_display_name(app)) : QString();
                first = false;
            } else if (id != currentId)
                mixed = true;
            if (app)
                g_object_unref(app);
        }
        auto choices = candidates(mimeTypes);
        int index = -1;
        for (int i = 0; i < choices.size(); ++i)
            if (!mixed && choices[i].toMap().value("id").toString() == currentId)
                index = i;
        m_roles.append(QVariantMap{{"key", key},
                                   {"types", mimeTypes.join(", ")},
                                   {"apps", choices},
                                   {"currentId", mixed ? QString() : currentId},
                                   {"currentName", mixed ? QString() : currentName},
                                   {"mixed", mixed},
                                   {"currentIndex", index}});
    }
    emit changed();
}
bool DefaultApps::setDefault(const QString& role, const QString& desktopId) {
    if (!types.contains(role)) {
        m_message = tr("Unknown application role.");
        emit changed();
        return false;
    }
    auto options = candidates(types.value(role));
    bool eligible = false;
    for (const auto& entry : options)
        if (entry.toMap().value("id").toString() == desktopId)
            eligible = true;
    if (!eligible) {
        m_message = tr("Choose an installed application that supports this file or link type.");
        emit changed();
        return false;
    }
    auto app = g_desktop_app_info_new(desktopId.toUtf8().constData());
    if (!app) {
        m_message = tr("The application is no longer installed.");
        emit changed();
        return false;
    }
    bool ok = true;
    for (const auto& mime : types.value(role)) {
        GError* error = nullptr;
        if (!g_app_info_set_as_default_for_type(G_APP_INFO(app), mime.toUtf8().constData(), &error)) {
            m_message = tr("Could not update %1: %2. Some associations may already have changed.")
                            .arg(mime, error ? QString::fromUtf8(error->message) : tr("Unknown error"));
            if (error)
                g_error_free(error);
            ok = false;
            break;
        }
    }
    g_object_unref(app);
    if (ok)
        m_message = tr("Default application updated.");
    refresh();
    return ok;
}
