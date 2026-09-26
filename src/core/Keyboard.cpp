#include "Keyboard.h"
#include "Command.h"
#include <QTimer>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusArgument>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QJsonDocument>
#include <QJsonObject>
Keyboard::Keyboard(QObject* parent) : QObject(parent) {
    QTimer::singleShot(0, this, &Keyboard::refresh);
    QTimer::singleShot(0, this, &Keyboard::refreshActive);
    auto bus = QDBusConnection::sessionBus();
    bus.connect("org.kde.keyboard", "/Layouts", "org.kde.KeyboardLayouts", "layoutChanged", this,
                SLOT(refreshActive()));
    bus.connect("org.kde.keyboard", "/Layouts", "org.kde.KeyboardLayouts", "layoutListChanged", this,
                SLOT(refreshActive()));
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Keyboard::refreshActive);
    timer->start(5000);
}
void Keyboard::refresh() {
    run({}, false);
}
void Keyboard::apply(QStringList layouts, QString shortcut) {
    if (layouts.isEmpty() || layouts.size() > 4)
        return;
    run({"--layouts", layouts.join(','), "--shortcut", shortcut}, true);
}
void Keyboard::run(QStringList args, bool applying) {
    if (working)
        return;
    working = true;
    status.clear();
    emit changed();
    auto command = new Command(this);
    connect(command, &Command::finished, this, [this, command, applying](bool ok, QString output) {
        working = false;
        command->deleteLater();
        QJsonParseError error;
        auto document = QJsonDocument::fromJson(output.toUtf8(), &error);
        if (!ok || error.error != QJsonParseError::NoError) {
            status = ok ? tr("Cannot read keyboard settings") : output;
            emit changed();
            return;
        }
        values = document.object().toVariantMap();
        if (applying) {
            if (qEnvironmentVariable("XDG_SESSION_DESKTOP").compare("harbor", Qt::CaseInsensitive) == 0 ||
                qEnvironmentVariableIsSet("HARBOR_USER_CONFIG")) {
                // Older KWin listens to reloadConfig; newer KWin uses KConfigWatcher.
                auto bus = QDBusConnection::sessionBus();
                auto signal = QDBusMessage::createSignal("/Layouts", "org.kde.keyboard", "reloadConfig");
                bus.send(signal);
                qDBusRegisterMetaType<QByteArrayList>();
                qDBusRegisterMetaType<QHash<QString, QByteArrayList>>();
                auto notification =
                    QDBusMessage::createSignal("/kxkbrc", "org.kde.kconfig.notify", "ConfigChanged");
                QHash<QString, QByteArrayList> changes{
                    {"Layout", {"Use", "LayoutList", "VariantList", "Options", "ResetOldOptions"}}};
                notification << QVariant::fromValue(changes);
                bus.send(notification);
                status = tr("Saved. Keyboard configuration reload requested.");
            } else
                status = tr("Saved for your next Harbor session.");
        }
        emit changed();
    });
    command->run("harbor-keyboard", args, 5000);
}
void Keyboard::switchNext() {
    auto msg = QDBusMessage::createMethodCall("org.kde.keyboard", "/Layouts", "org.kde.KeyboardLayouts",
                                              "switchToNextLayout");
    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(msg, 2000), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
        QDBusPendingReply<> reply = *watcher;
        status = reply.isError() ? tr("Keyboard switching is available inside the Harbor session.")
                                 : tr("Switched input source.");
        watcher->deleteLater();
        refreshActive();
        emit changed();
    });
}

void Keyboard::clearActive() {
    liveLayouts.clear();
    liveIndex = -1;
    liveAvailable = false;
    label = QString::fromUtf8("⌨");
    layoutName.clear();
    emit activeChanged();
}
void Keyboard::refreshActive() {
    const uint generation = ++liveGeneration;
    auto message = QDBusMessage::createMethodCall("org.kde.keyboard", "/Layouts", "org.kde.KeyboardLayouts",
                                                  "getLayoutsList");
    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message, 2000), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, generation] {
        const auto reply = watcher->reply();
        watcher->deleteLater();
        if (generation != liveGeneration)
            return;
        if (reply.type() == QDBusMessage::ErrorMessage || reply.arguments().isEmpty()) {
            clearActive();
            return;
        }
        QList<QStringList> layouts;
        const auto argument = qvariant_cast<QDBusArgument>(reply.arguments().first());
        argument.beginArray();
        while (!argument.atEnd()) {
            QString code, variant, name;
            argument.beginStructure();
            argument >> code >> variant >> name;
            argument.endStructure();
            layouts.append({code, variant, name});
        }
        argument.endArray();
        auto message = QDBusMessage::createMethodCall("org.kde.keyboard", "/Layouts",
                                                      "org.kde.KeyboardLayouts", "getLayout");
        auto current =
            new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message, 2000), this);
        connect(current, &QDBusPendingCallWatcher::finished, this, [this, current, generation, layouts] {
            QDBusPendingReply<uint> reply = *current;
            current->deleteLater();
            if (generation != liveGeneration)
                return;
            if (reply.isError() || reply.value() >= uint(layouts.size())) {
                clearActive();
                return;
            }
            const auto row = layouts.at(reply.value());
            const auto code = row.at(0);
            label = code == "ara"                    ? QString::fromUtf8("ع")
                    : (code == "us" || code == "gb") ? QString("EN")
                    : code.isEmpty()                 ? QString::fromUtf8("⌨")
                                                     : code.toUpper();
            layoutName = row.at(2);
            if (!row.at(1).isEmpty())
                layoutName += " (" + row.at(1) + ")";
            liveLayouts.clear();
            liveIndex = int(reply.value());
            for (int i = 0; i < layouts.size(); ++i) {
                const auto item = layouts.at(i);
                auto code = item.at(0);
                QString shortName = code == "ara"                    ? QString::fromUtf8("ع")
                                    : (code == "us" || code == "gb") ? QString("EN")
                                                                     : code.toUpper();
                liveLayouts.append(QVariantMap{{"index", i},
                                               {"code", code},
                                               {"variant", item.at(1)},
                                               {"name", item.at(2)},
                                               {"label", shortName}});
            }
            liveAvailable = true;
            emit activeChanged();
        });
    });
}

void Keyboard::selectLayout(int index) {
    if (!liveAvailable || index < 0 || index >= liveLayouts.size()) {
        status = tr("Input source is no longer available. Refresh the list.");
        emit changed();
        emit selectionFinished(false);
        return;
    }
    auto message = QDBusMessage::createMethodCall("org.kde.keyboard", "/Layouts", "org.kde.KeyboardLayouts",
                                                  "setLayout");
    message << uint(index);
    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message, 2000), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
        QDBusPendingReply<bool> reply = *watcher;
        const bool ok = !reply.isError() && reply.value();
        status = ok ? QString() : tr("Could not select the input source. Refresh and try again.");
        watcher->deleteLater();
        refreshActive();
        emit changed();
        emit selectionFinished(ok);
    });
}
