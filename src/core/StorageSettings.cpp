#include "StorageSettings.h"
#include <QStorageInfo>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include <QSet>
#include <QThread>
#include <memory>
StorageSettings::StorageSettings(QObject* parent) : QObject(parent) {}
void StorageSettings::refresh() {
    if (m_busy)
        return;
    m_busy = true;
    emit changed();
    auto result = std::make_shared<QVariantList>();
    auto* worker = QThread::create([result] {
        QVariantList volumes;
        QSet<QString> seen;
        for (auto volume : QStorageInfo::mountedVolumes()) {
            const auto mount = volume.rootPath();
            if (QSet<QByteArray>{"tmpfs", "devtmpfs", "proc", "sysfs", "cgroup", "cgroup2", "debugfs",
                                 "tracefs", "securityfs", "pstore", "efivarfs", "configfs", "fusectl",
                                 "mqueue", "hugetlbfs"}
                    .contains(volume.fileSystemType()))
                continue;
            if (!volume.isValid() || !volume.isReady() || volume.bytesTotal() <= 0 || seen.contains(mount))
                continue;
            seen.insert(mount);
            const qint64 total = volume.bytesTotal(), free = qBound<qint64>(0, volume.bytesFree(), total),
                         available = qBound<qint64>(0, volume.bytesAvailable(), total);
            volumes.append(
                QVariantMap{{"mount", mount},
                            {"name", volume.displayName().isEmpty() ? mount : volume.displayName()},
                            {"filesystem", QString::fromUtf8(volume.fileSystemType())},
                            {"total", total},
                            {"used", total - free},
                            {"available", available},
                            {"fraction", double(total - free) / double(total)},
                            {"readOnly", volume.isReadOnly()}});
        }
        *result = volumes;
    });
    connect(worker, &QThread::finished, this, [this, result] {
        m_volumes = *result;
        m_busy = false;
        m_error.clear();
        emit changed();
    });
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
}
bool StorageSettings::openVolume(const QString& mount) {
    bool known = false;
    for (const auto& v : m_volumes)
        if (v.toMap().value("mount").toString() == mount)
            known = true;
    // Use the last worker snapshot: stat calls can block on disconnected network mounts.
    // The file manager handles a volume disappearing after that snapshot.
    if (!known || !QDir::isAbsolutePath(mount)) {
        m_error = tr("This volume is no longer available. Refresh and try again.");
        emit changed();
        return false;
    }
    const bool opened = QDesktopServices::openUrl(QUrl::fromLocalFile(mount));
    m_error = opened ? QString() : tr("The file manager could not be opened.");
    emit changed();
    return opened;
}

#include <QFileInfo>
#include <QDirIterator>
#include <QMap>
#include <QFile>
#include <sys/stat.h>
#include <algorithm>
StorageSettings::~StorageSettings() {
    cancelAnalysis();
}
QString StorageSettings::homePath() const {
    return QDir::homePath();
}
void StorageSettings::cancelAnalysis() {
    if (m_cancel)
        m_cancel->store(true);
}
void StorageSettings::analyze(QString path) {
    if (m_analyzing)
        return;
    if (!QDir::isAbsolutePath(path)) {
        m_analysisMessage = tr("Choose an absolute folder path.");
        emit analysisChanged();
        return;
    }
    m_analyzing = true;
    m_analysisMessage = tr("Analyzing…");
    m_categories.clear();
    m_largest.clear();
    m_cancel = std::make_shared<std::atomic_bool>(false);
    emit analysisChanged();
    struct Result {
        QString error;
        QVariantList categories, largest;
        quint64 count = 0, skipped = 0, entries = 0;
        bool canceled = false, capped = false;
    };
    auto result = std::make_shared<Result>();
    auto cancel = m_cancel;
    auto worker = QThread::create([path, result, cancel] {
        QFileInfo info(path);
        QString root = info.canonicalFilePath();
        if (cancel->load()) {
            result->canceled = true;
            return;
        }
        if (!info.isDir() || info.isSymLink() || root.isEmpty() || root == "/proc" ||
            root.startsWith("/proc/") || root == "/sys" || root.startsWith("/sys/") || root == "/dev" ||
            root.startsWith("/dev/") || root == "/run" || root.startsWith("/run/")) {
            result->error = QObject::tr(
                "Choose a readable local folder, not a system device directory or symbolic link.");
            return;
        }
        struct stat rootStat{};
        if (::lstat(QFile::encodeName(root).constData(), &rootStat) != 0) {
            result->error = QObject::tr("The folder could not be read.");
            return;
        }
        auto device = rootStat.st_dev;
        QStringList dirs{root};
        QSet<QString> seen, seenDirs{QString::number(qulonglong(rootStat.st_dev)) + ":" +
                                     QString::number(qulonglong(rootStat.st_ino))};
        QMap<QString, qint64> totals;
        QList<QPair<qint64, QString>> largest;
        const QSet<QString> pictures{"png", "jpg", "jpeg", "gif", "webp", "svg", "heic", "raw"},
            videos{"mp4", "mkv", "webm", "mov", "avi"}, audio{"mp3", "flac", "wav", "ogg", "m4a"},
            docs{"pdf", "doc", "docx", "odt", "txt", "md", "xls", "xlsx", "ppt", "pptx"},
            archives{"zip", "gz", "xz", "7z", "tar", "deb", "rpm", "appimage"};
        while (!dirs.isEmpty() && !cancel->load() && !result->capped) {
            auto dir = dirs.takeLast();
            QDir folder(dir);
            if (!QFileInfo(dir).isReadable()) {
                ++result->skipped;
                continue;
            }
            QDirIterator iterator(dir, QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
            while (iterator.hasNext()) {
                iterator.next();
                auto file = iterator.fileInfo();
                if (cancel->load())
                    break;
                if (++result->entries > 1000000) {
                    result->capped = true;
                    break;
                }
                struct stat st{};
                if (::lstat(QFile::encodeName(file.absoluteFilePath()).constData(), &st) != 0) {
                    ++result->skipped;
                    continue;
                }
                if (st.st_dev != device || S_ISLNK(st.st_mode))
                    continue;
                if (S_ISDIR(st.st_mode)) {
                    auto dirKey =
                        QString::number(qulonglong(st.st_dev)) + ":" + QString::number(qulonglong(st.st_ino));
                    if (!seenDirs.contains(dirKey)) {
                        seenDirs.insert(dirKey);
                        dirs << file.absoluteFilePath();
                    }
                    continue;
                }
                if (!S_ISREG(st.st_mode))
                    continue;
                auto key =
                    QString::number(qulonglong(st.st_dev)) + ":" + QString::number(qulonglong(st.st_ino));
                if (seen.contains(key))
                    continue;
                seen.insert(key);
                ++result->count;
                QString ext = file.suffix().toLower(), category = pictures.contains(ext) ? "Pictures"
                                                                  : videos.contains(ext) ? "Videos"
                                                                  : audio.contains(ext)  ? "Audio"
                                                                  : docs.contains(ext)   ? "Documents"
                                                                  : archives.contains(ext)
                                                                      ? "Archives and packages"
                                                                      : "Other";
                qint64 allocated = qMax<qint64>(0, st.st_blocks) * 512;
                totals[category] += allocated;
                largest.append({allocated, file.absoluteFilePath()});
                std::sort(largest.begin(), largest.end(), [](const auto& a, const auto& b) {
                    return a.first > b.first;
                });
                if (largest.size() > 20)
                    largest.removeLast();
                if (result->count >= 1000000) {
                    result->capped = true;
                    break;
                }
            }
        }
        result->canceled = cancel->load();
        for (auto it = totals.begin(); it != totals.end(); ++it)
            result->categories.append(QVariantMap{{"name", it.key()}, {"bytes", it.value()}});
        for (auto file : largest)
            result->largest.append(QVariantMap{{"path", file.second}, {"bytes", file.first}});
    });
    connect(worker, &QThread::finished, this, [this, result] {
        m_analyzing = false;
        m_categories = result->categories;
        m_largest = result->largest;
        m_analysisMessage = (result->canceled ? tr("Canceled. Partial results: ")
                             : result->capped ? tr("Scan limit reached. Partial results: ")
                                              : tr("Finished: ")) +
                            tr("%1 files; %2 unreadable entries.").arg(result->count).arg(result->skipped);
        if (!result->error.isEmpty())
            m_analysisMessage = result->error;
        emit analysisChanged();
    });
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
}
