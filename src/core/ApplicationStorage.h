#pragma once
#include <QObject>
#include <QVariantMap>
#include <QVariantList>
#include <QTimer>
#include <memory>
struct ApplicationStorageProgress;
class ApplicationStorage:public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantList applications READ applications NOTIFY changed)
 Q_PROPERTY(QVariantList associations READ associations NOTIFY changed)
 Q_PROPERTY(QVariantMap summary READ summary NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
 Q_PROPERTY(qulonglong scannedFiles READ scannedFiles NOTIFY changed)
 Q_PROPERTY(QString message READ message NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
public:
 struct Options {QString home,dataHome,cacheHome,configHome,associationsFile,dpkgProgram="dpkg-query",flatpakProgram="flatpak";QStringList applicationDirs;quint64 maximumEntries=500000;int commandTimeoutMs=15000;};
 explicit ApplicationStorage(QObject *parent=nullptr);
 explicit ApplicationStorage(const Options &options,QObject *parent=nullptr);
 ~ApplicationStorage();
 QVariantList applications()const{return m_applications;}QVariantList associations()const{return m_associations;}QVariantMap summary()const{return m_summary;}
 bool busy()const{return m_busy;}qulonglong scannedFiles()const;QString message()const{return m_message;}QString error()const{return m_error;}
 Q_INVOKABLE void measure();Q_INVOKABLE void cancel();
 Q_INVOKABLE void associateFolder(const QString &id,const QString &path,const QString &kind);
 Q_INVOKABLE void removeAssociation(const QString &id,const QString &path);
 static QVariantMap measureRoots(const QVariantList &roots,quint64 maximumEntries=500000);
signals:void changed();
private:
 void start(const QVariantList &associations,bool saveAssociations=false);
 Options m_options;QVariantList m_applications,m_associations;QVariantMap m_summary;bool m_busy=false;QString m_message,m_error;
 QTimer m_progressTimer;std::shared_ptr<ApplicationStorageProgress> m_progress;
};
