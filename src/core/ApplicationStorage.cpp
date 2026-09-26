#include "ApplicationStorage.h"
#include <QDir>
#include <QFile>
#include <QHash>
#include <QSet>
#include <atomic>
#include <memory>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
struct ApplicationStorageProgress {
    std::atomic_bool cancel{false};
    std::atomic<quint64> entries{0}, files{0};
    std::atomic_int phase{0};
};
namespace {
using ScanProgress = ApplicationStorageProgress;
struct FileAllocation {
    qint64 bytes = 0;
    QSet<QString> references;
};
bool beneath(const QString& path, const QString& root) {
    return path == root || path.startsWith(root + "/");
}
int openPath(const QString& path) {
    if (!QDir::isAbsolutePath(path) || QDir::cleanPath(path) != path)
        return -1;
    int fd = ::open("/", O_PATH | O_DIRECTORY | O_CLOEXEC);
    const auto components = path.split('/', Qt::SkipEmptyParts);
    for (int i = 0; i < components.size() && fd >= 0; ++i) {
        const auto name = QFile::encodeName(components[i]);
        int next = ::openat(fd, name.constData(),
                            O_PATH | O_NOFOLLOW | O_CLOEXEC | (i + 1 < components.size() ? O_DIRECTORY : 0));
        ::close(fd);
        fd = next;
    }
    return fd;
}
class Measurer {
public:
    quint64 limit, skipped = 0;
    bool capped = false;
    QHash<QString, FileAllocation> files;
    QSet<QString> applicationIds;
    std::shared_ptr<ScanProgress> progress;
    QVariantList associations;
    explicit Measurer(quint64 maximum, std::shared_ptr<ScanProgress> state)
        : limit(maximum), progress(std::move(state)) {}
    bool stop() {
        if (progress->cancel.load())
            return true;
        if (progress->entries.load() >= limit) {
            capped = true;
            return true;
        }
        return false;
    }
    void record(const struct stat& st, QString path, QStringList owners, QString kind) {
        for (const auto& value : associations) {
            const auto association = value.toMap();
            if (beneath(path, association.value("path").toString())) {
                owners = {association.value("id").toString()};
                kind = association.value("kind").toString();
                break;
            }
        }
        if (owners.isEmpty())
            owners << QString();
        const auto key =
            QString::number(qulonglong(st.st_dev)) + ":" + QString::number(qulonglong(st.st_ino));
        auto& file = files[key];
        file.bytes = qMax<qint64>(0, st.st_blocks) * 512;
        for (const auto& owner : owners) {
            file.references.insert(owner + "|" + kind);
            if (!owner.isEmpty())
                applicationIds.insert(owner);
        }
        progress->files.store(files.size());
    }
    void directory(int fd, const QString& path, dev_t device, const QStringList& owners, const QString& kind,
                   const QStringList& excluded, int depth) {
        if (depth > 64) {
            ++skipped;
            ::close(fd);
            return;
        }
        DIR* directory = ::fdopendir(fd);
        if (!directory) {
            ++skipped;
            ::close(fd);
            return;
        }
        while (!stop()) {
            errno = 0;
            auto* entry = ::readdir(directory);
            if (!entry) {
                if (errno)
                    ++skipped;
                break;
            }
            const QByteArray name(entry->d_name);
            if (name == "." || name == "..")
                continue;
            ++progress->entries;
            const auto child = path + "/" + QFile::decodeName(name);
            bool ignore = false;
            for (const auto& root : excluded)
                if (beneath(child, root)) {
                    ignore = true;
                    break;
                }
            if (ignore)
                continue;
            struct stat st{};
            if (::fstatat(fd, name.constData(), &st, AT_SYMLINK_NOFOLLOW)) {
                ++skipped;
                continue;
            }
            if (st.st_dev != device || S_ISLNK(st.st_mode)) {
                ++skipped;
                continue;
            }
            if (S_ISREG(st.st_mode))
                record(st, child, owners, kind);
            else if (S_ISDIR(st.st_mode)) {
                int next = ::openat(fd, name.constData(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
                if (next < 0) {
                    ++skipped;
                    continue;
                }
                struct stat actual{};
                if (::fstat(next, &actual) || actual.st_dev != device || actual.st_ino != st.st_ino) {
                    ::close(next);
                    ++skipped;
                    continue;
                }
                this->directory(next, child, device, owners, kind, excluded, depth + 1);
            }
        }
        ::closedir(directory);
    }
    void root(const QVariantMap& root) {
        if (stop())
            return;
        const auto path = root.value("path").toString();
        const auto owners = root.value("owners").toStringList();
        const auto kind = root.value("kind").toString();
        for (const auto& id : owners)
            if (!id.isEmpty())
                applicationIds.insert(id);
        if (!QStringList{"installed", "data", "cache"}.contains(kind)) {
            ++skipped;
            return;
        }
        if (path == "/" || beneath(path, "/proc") || beneath(path, "/sys") || beneath(path, "/dev") ||
            beneath(path, "/run")) {
            ++skipped;
            return;
        }
        int fd = openPath(path);
        if (fd < 0) {
            ++skipped;
            return;
        }
        struct stat st{};
        if (::fstat(fd, &st)) {
            ::close(fd);
            ++skipped;
            return;
        }
        if (S_ISREG(st.st_mode)) {
            ++progress->entries;
            record(st, path, owners, kind);
        } else if (S_ISDIR(st.st_mode) && !root.value("fileOnly").toBool()) {
            int readable = ::openat(fd, ".", O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
            if (readable < 0)
                ++skipped;
            else
                directory(readable, path, st.st_dev, owners, kind, root.value("excluded").toStringList(), 0);
        } else if (S_ISLNK(st.st_mode))
            ++skipped;
        ::close(fd);
    }
    QVariantMap result() {
        QMap<QString, QVariantMap> applications;
        for (const auto& id : applicationIds)
            applications[id] = {{"id", id},
                                {"installedBytes", qint64(0)},
                                {"dataBytes", qint64(0)},
                                {"cacheBytes", qint64(0)},
                                {"sharedReferencedBytes", qint64(0)},
                                {"totalBytes", qint64(0)}};
        qint64 total = 0, shared = 0, unattributedData = 0, unattributedCache = 0;
        for (const auto& file : files) {
            total += file.bytes;
            QSet<QString> owners, kinds;
            for (const auto& reference : file.references) {
                owners.insert(reference.section('|', 0, 0));
                kinds.insert(reference.section('|', 1, 1));
            }
            if (owners.size() != 1) {
                shared += file.bytes;
                for (const auto& owner : owners)
                    if (!owner.isEmpty())
                        applications[owner]["sharedReferencedBytes"] =
                            applications[owner].value("sharedReferencedBytes").toLongLong() + file.bytes;
                continue;
            }
            // A hardlink spanning categories within one app still belongs to that app.
            const QString owner = *owners.begin(), kind = kinds.contains("installed") ? "installed"
                                                          : kinds.contains("data")    ? "data"
                                                                                      : "cache";
            if (owner.isEmpty()) {
                if (kind == "cache")
                    unattributedCache += file.bytes;
                else
                    unattributedData += file.bytes;
                continue;
            }
            auto& row = applications[owner];
            row[kind + "Bytes"] = row.value(kind + "Bytes").toLongLong() + file.bytes;
            row["totalBytes"] = row.value("totalBytes").toLongLong() + file.bytes;
        }
        QVariantList rows;
        for (const auto& row : applications)
            rows << row;
        return {{"apps", rows},
                {"measuredBytes", total},
                {"sharedBytes", shared},
                {"unattributedDataBytes", unattributedData},
                {"unattributedCacheBytes", unattributedCache},
                {"scannedFiles", qulonglong(files.size())},
                {"skipped", qulonglong(skipped)},
                {"capped", capped},
                {"canceled", progress->cancel.load()}};
    }
};
} // namespace
QVariantMap ApplicationStorage::measureRoots(const QVariantList& roots, quint64 maximumEntries) {
    Measurer scan(maximumEntries, std::make_shared<ScanProgress>());
    for (const auto& root : roots)
        scan.root(root.toMap());
    return scan.result();
}
#include <QStandardPaths>
#include <QSettings>
#include <QThread>
#include <QProcess>
#include <QProcessEnvironment>
#include <QElapsedTimer>
#include <QDirIterator>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSaveFile>
namespace {
ApplicationStorage::Options defaults() {
    ApplicationStorage::Options o;
    o.home = QDir::homePath();
    o.dataHome = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    o.cacheHome = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
    o.configHome = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    o.associationsFile = o.configHome + "/harbor/application-storage.json";
    for (const auto& path : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation))
        o.applicationDirs << path;
    return o;
}
struct CommandOutput {
    bool ok = false;
    QByteArray output;
};
CommandOutput command(const QString& program, const QStringList& arguments,
                      const ApplicationStorage::Options& o, const std::shared_ptr<ScanProgress>& progress) {
    CommandOutput result;
    if (progress->cancel.load())
        return result;
    QProcess process;
    auto env = QProcessEnvironment::systemEnvironment();
    env.insert("LC_ALL", "C.UTF-8");
    process.setProcessEnvironment(env);
    process.setProgram(program);
    process.setArguments(arguments);
    process.start();
    if (!process.waitForStarted(1000))
        return result;
    QElapsedTimer timer;
    timer.start();
    while (process.state() != QProcess::NotRunning) {
        process.waitForFinished(50);
        result.output += process.readAllStandardOutput();
        process.readAllStandardError();
        if (progress->cancel.load() || timer.elapsed() > o.commandTimeoutMs ||
            result.output.size() > 16 * 1024 * 1024) {
            process.kill();
            process.waitForFinished(1000);
            return result;
        }
    }
    result.output += process.readAllStandardOutput();
    result.ok = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
    return result;
}
struct Inventory {
    QMap<QString, QVariantMap> applications;
    QVariantList roots;
    bool limited = false;
};
Inventory inventory(const ApplicationStorage::Options& o, const std::shared_ptr<ScanProgress>& progress) {
    Inventory result;
    QMap<QString, QString> desktopOwners;
    QSet<QString> seen;
    QElapsedTimer overall;
    overall.start();
    progress->phase = 0;
    for (const auto& directory : o.applicationDirs) {
        QDirIterator iterator(directory, {"*.desktop"}, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext() && !progress->cancel.load()) {
            if (overall.elapsed() > 60000) {
                result.limited = true;
                break;
            }
            const auto source = iterator.next();
            const auto file = iterator.fileInfo();
            QString desktopId = QDir(directory).relativeFilePath(source).replace('/', '-');
            if (seen.contains(desktopId))
                continue;
            seen.insert(desktopId);
            if (!QRegularExpression("^[A-Za-z0-9][A-Za-z0-9._+-]*\\.desktop$").match(desktopId).hasMatch() ||
                file.size() > 65536)
                continue;
            QSettings data(source, QSettings::IniFormat);
            data.beginGroup("Desktop Entry");
            if (data.value("Type").toString() != "Application" || data.value("Hidden", false).toBool() ||
                data.value("NoDisplay", false).toBool() || data.contains("X-Flatpak"))
                continue;
            const QString id = "desktop:" + desktopId;
            result.applications[id] = {{"id", id},
                                       {"name", data.value("Name", desktopId).toString()},
                                       {"type", "native"},
                                       {"installedKnown", false},
                                       {"dataKnown", false},
                                       {"cacheKnown", false},
                                       {"packages", QStringList{}}};
            desktopOwners[source] = id;
            if (result.applications.size() >= 512 || overall.elapsed() > 60000) {
                result.limited = true;
                break;
            }
        }
        if (result.limited)
            break;
    }
    QMap<QString, QStringList> packages;
    const auto desktops = desktopOwners.keys();
    for (int first = 0; first < desktops.size() && !progress->cancel.load(); first += 128) {
        QStringList args{"-S", "--"};
        args += desktops.mid(first, 128);
        const auto response = command(o.dpkgProgram, args, o, progress);
        for (const auto& line : QString::fromUtf8(response.output).split('\n')) {
            const int separator = line.indexOf(": ");
            if (separator < 0)
                continue;
            const QString source = line.mid(separator + 2);
            if (!desktopOwners.contains(source))
                continue;
            const auto id = desktopOwners.value(source);
            for (auto package : line.left(separator).split(',')) {
                package = package.trimmed();
                if (!QRegularExpression("^[a-z0-9][a-z0-9+.-]*(?::[a-z0-9-]+)?$").match(package).hasMatch())
                    continue;
                if (!packages[package].contains(id))
                    packages[package] << id;
            }
        }
    }
    progress->phase = 1;
    int count = 0;
    for (auto package = packages.begin(); package != packages.end() && !progress->cancel.load(); ++package) {
        if (++count > 512 || overall.elapsed() > 60000) {
            result.limited = true;
            break;
        }
        const auto response = command(o.dpkgProgram, {"-L", "--", package.key()}, o, progress);
        if (!response.ok)
            continue;
        for (const auto& id : package.value()) {
            result.applications[id]["installedKnown"] = true;
            auto names = result.applications[id].value("packages").toStringList();
            names << package.key();
            result.applications[id]["packages"] = names;
        }
        for (const auto& path : QString::fromUtf8(response.output).split('\n', Qt::SkipEmptyParts)) {
            if (result.roots.size() >= qMin<quint64>(o.maximumEntries, 500000)) {
                result.limited = true;
                break;
            }
            if (QDir::isAbsolutePath(path))
                result.roots << QVariantMap{{"path", QDir::cleanPath(path)},
                                            {"fileOnly", true},
                                            {"owners", package.value()},
                                            {"kind", "installed"}};
        }
        if (result.limited)
            break;
    }
    const auto flatpak =
        command(o.flatpakProgram, {"list", "--app", "--columns=application,name,installation"}, o, progress);
    QMap<QString, QStringList> flatpakOwners;
    if (flatpak.ok)
        for (const auto& line : QString::fromUtf8(flatpak.output).split('\n', Qt::SkipEmptyParts)) {
            const auto fields = line.split('\t');
            if (fields.size() < 3)
                continue;
            const auto appId = fields[0], installation = fields[2];
            if (!QRegularExpression("^[A-Za-z0-9_-]+(?:\\.[A-Za-z0-9_-]+){2,}$").match(appId).hasMatch() ||
                !QRegularExpression("^[A-Za-z0-9_.-]+$").match(installation).hasMatch())
                continue;
            const QString id = "flatpak:" + installation + ":" + appId;
            result.applications[id] = {{"id", id},          {"name", fields[1] + " (" + installation + ")"},
                                       {"type", "flatpak"}, {"installedKnown", false},
                                       {"dataKnown", true}, {"cacheKnown", true}};
            flatpakOwners[appId] << id;
            const auto location = command(o.flatpakProgram,
                                          {"info",
                                           installation == "user"     ? "--user"
                                           : installation == "system" ? "--system"
                                                                      : "--installation=" + installation,
                                           "--show-location", appId},
                                          o, progress);
            if (location.ok) {
                const auto path = QString::fromUtf8(location.output).trimmed() + "/files";
                int fd = openPath(path);
                struct stat st{};
                if (fd >= 0 && ::fstat(fd, &st) == 0 && S_ISDIR(st.st_mode)) {
                    result.roots << QVariantMap{
                        {"path", path}, {"owners", QStringList{id}}, {"kind", "installed"}};
                    result.applications[id]["installedKnown"] = true;
                }
                if (fd >= 0)
                    ::close(fd);
            }
            if (result.applications.size() >= 1024 || overall.elapsed() > 60000) {
                result.limited = true;
                break;
            }
        }
    for (auto app = flatpakOwners.begin(); app != flatpakOwners.end(); ++app)
        for (const auto& sub : QStringList{"data", "config", "cache"})
            result.roots << QVariantMap{{"path", o.home + "/.var/app/" + app.key() + "/" + sub},
                                        {"owners", app.value()},
                                        {"kind", sub == "cache" ? "cache" : "data"}};
    result.roots << QVariantMap{{"path", o.dataHome},
                                {"kind", "data"},
                                {"excluded", QStringList{o.dataHome + "/flatpak"}}}
                 << QVariantMap{{"path", o.configHome}, {"kind", "data"}}
                 << QVariantMap{{"path", o.cacheHome}, {"kind", "cache"}};
    return result;
}
QVariantList loadAssociations(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 1024 * 1024)
        return {};
    const auto document = QJsonDocument::fromJson(file.readAll());
    return document.isArray() ? document.array().toVariantList() : QVariantList{};
}
bool validAssociation(const QVariantMap& row, const ApplicationStorage::Options& o) {
    const auto path = row.value("path").toString();
    return !row.value("id").toString().isEmpty() &&
           QStringList{"data", "cache"}.contains(row.value("kind").toString()) && path != o.home &&
           beneath(path, o.home) && QDir::cleanPath(path) == path && !beneath(o.dataHome, path) &&
           !beneath(o.cacheHome, path) && !beneath(o.configHome, path);
}
} // namespace
ApplicationStorage::ApplicationStorage(QObject* parent) : ApplicationStorage(defaults(), parent) {}
ApplicationStorage::ApplicationStorage(const Options& options, QObject* parent)
    : QObject(parent), m_options(options) {
    m_progressTimer.setInterval(200);
    connect(&m_progressTimer, &QTimer::timeout, this, [this] {
        if (m_progress) {
            const int phase = m_progress->phase.load();
            m_message = phase == 0   ? tr("Finding installed applications…")
                        : phase == 1 ? tr("Reading installed file lists…")
                                     : tr("Measuring allocated space… %1 files").arg(scannedFiles());
            emit changed();
        }
    });
}
ApplicationStorage::~ApplicationStorage() {
    cancel();
}
qulonglong ApplicationStorage::scannedFiles() const {
    return m_progress ? m_progress->files.load() : m_summary.value("scannedFiles").toULongLong();
}
void ApplicationStorage::measure() {
    if (!m_busy)
        start({}, false);
}
void ApplicationStorage::cancel() {
    if (m_progress)
        m_progress->cancel = true;
}
void ApplicationStorage::associateFolder(const QString& id, const QString& path, const QString& kind) {
    if (m_busy)
        return;
    bool known = false;
    for (const auto& app : m_applications)
        if (app.toMap().value("id") == id)
            known = true;
    QVariantMap row{{"id", id}, {"path", QDir::cleanPath(path)}, {"kind", kind}};
    if (!known || !validAssociation(row, m_options)) {
        m_error = tr(
            "Choose a listed application and an app-specific folder inside your home, not a shared XDG root.");
        emit changed();
        return;
    }
    for (const auto& v : m_associations) {
        const auto existing = v.toMap();
        const auto other = existing.value("path").toString();
        if (beneath(row.value("path").toString(), other) || beneath(other, row.value("path").toString())) {
            m_error = tr("This folder overlaps an existing association. Remove that association first.");
            emit changed();
            return;
        }
    }
    const auto flatpakHome = m_options.home + "/.var/app";
    if (beneath(row.value("path").toString(), flatpakHome) ||
        beneath(row.value("path").toString(), m_options.dataHome + "/flatpak")) {
        m_error = tr("Flatpak data folders are associated automatically.");
        emit changed();
        return;
    }
    auto next = m_associations;
    next << row;
    start(next, true);
}
void ApplicationStorage::removeAssociation(const QString& id, const QString& path) {
    if (m_busy)
        return;
    auto next = m_associations;
    for (int i = next.size() - 1; i >= 0; --i)
        if (next[i].toMap().value("id") == id && next[i].toMap().value("path") == path)
            next.removeAt(i);
    start(next, true);
}
void ApplicationStorage::start(const QVariantList& proposed, bool saveAssociations) {
    m_busy = true;
    m_error.clear();
    m_message = tr("Finding installed applications…");
    m_progress = std::make_shared<ScanProgress>();
    m_progressTimer.start();
    emit changed();
    const auto progress = m_progress;
    const auto options = m_options;
    struct Result {
        QVariantList apps, associations;
        QVariantMap summary;
        QString error;
    };
    auto result = std::make_shared<Result>();
    auto* worker = QThread::create([options, progress, result, proposed, saveAssociations] {
        auto associations = saveAssociations ? proposed : loadAssociations(options.associationsFile);
        QVariantList valid;
        for (const auto& v : associations) {
            const auto row = v.toMap();
            if (!validAssociation(row, options))
                continue;
            if (saveAssociations) {
                int fd = openPath(row.value("path").toString());
                struct stat st{};
                bool directory = fd >= 0 && ::fstat(fd, &st) == 0 && S_ISDIR(st.st_mode);
                if (directory) {
                    int readable = ::openat(fd, ".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
                    directory = readable >= 0;
                    if (readable >= 0)
                        ::close(readable);
                }
                if (fd >= 0)
                    ::close(fd);
                if (!directory) {
                    result->error =
                        QObject::tr("Choose a readable folder without symbolic links in its path.");
                    return;
                }
            }
            valid << row;
        }
        if (saveAssociations) {
            QDir().mkpath(QFileInfo(options.associationsFile).absolutePath());
            QSaveFile file(options.associationsFile);
            if (!file.open(QIODevice::WriteOnly) ||
                file.write(QJsonDocument(QJsonArray::fromVariantList(valid)).toJson()) < 0 ||
                !file.commit()) {
                result->error = QObject::tr("The folder associations could not be saved.");
                return;
            }
        }
        result->associations = valid;
        auto found = inventory(options, progress);
        Measurer scan(options.maximumEntries, progress);
        for (const auto& v : valid) {
            const auto row = v.toMap();
            const auto id = row.value("id").toString();
            if (!found.applications.contains(id))
                continue;
            scan.associations << row;
            const auto path = row.value("path").toString();
            if (!beneath(path, options.dataHome) && !beneath(path, options.configHome) &&
                !beneath(path, options.cacheHome))
                found.roots << QVariantMap{
                    {"path", row.value("path")}, {"owners", QStringList{id}}, {"kind", row.value("kind")}};
            found.applications[id][row.value("kind").toString() + "Known"] = true;
        }
        progress->phase = 2;
        for (const auto& root : found.roots) {
            if (progress->cancel.load())
                break;
            scan.root(root.toMap());
        }
        result->summary = scan.result();
        result->summary["inventoryLimited"] = found.limited;
        QMap<QString, QVariantMap> measured;
        for (const auto& v : result->summary.value("apps").toList())
            measured[v.toMap().value("id").toString()] = v.toMap();
        for (auto app = found.applications.begin(); app != found.applications.end(); ++app) {
            auto row = app.value();
            const auto values = measured.value(app.key());
            for (auto value = values.begin(); value != values.end(); ++value)
                row[value.key()] = value.value();
            for (const auto& key : QStringList{"installedBytes", "dataBytes", "cacheBytes",
                                               "sharedReferencedBytes", "totalBytes"})
                if (!row.contains(key))
                    row[key] = qint64(0);
            result->apps << row;
        }
        std::sort(result->apps.begin(), result->apps.end(), [](const QVariant& a, const QVariant& b) {
            return a.toMap().value("totalBytes").toLongLong() > b.toMap().value("totalBytes").toLongLong();
        });
        result->summary.remove("apps");
    });
    connect(worker, &QThread::finished, this, [this, result] {
        m_progressTimer.stop();
        m_busy = false;
        if (!result->error.isEmpty())
            m_error = result->error;
        else {
            m_applications = result->apps;
            m_associations = result->associations;
            m_summary = result->summary;
            m_message = m_summary.value("canceled").toBool() ? tr("Canceled. These are partial results.")
                        : (m_summary.value("capped").toBool() || m_summary.value("inventoryLimited").toBool())
                            ? tr("Scan limit reached. These are partial results.")
                            : tr("Measurement complete: %1 unique files.")
                                  .arg(m_summary.value("scannedFiles").toULongLong());
        }
        emit changed();
    });
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
}
