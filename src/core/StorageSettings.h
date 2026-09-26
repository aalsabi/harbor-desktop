#pragma once
#include <QObject>
#include <atomic>
#include <memory>
#include <QVariantList>
class StorageSettings : public QObject {
 Q_OBJECT
 Q_PROPERTY(bool analyzing READ analyzing NOTIFY analysisChanged)
 Q_PROPERTY(QVariantList categories READ categories NOTIFY analysisChanged)
 Q_PROPERTY(QVariantList largestFiles READ largestFiles NOTIFY analysisChanged)
 Q_PROPERTY(QString analysisMessage READ analysisMessage NOTIFY analysisChanged)
 Q_PROPERTY(QString homePath READ homePath CONSTANT)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
 Q_PROPERTY(QVariantList volumes READ volumes NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
public:
 explicit StorageSettings(QObject *parent=nullptr);
 ~StorageSettings();
 bool analyzing()const{return m_analyzing;}QVariantList categories()const{return m_categories;}QVariantList largestFiles()const{return m_largest;}QString analysisMessage()const{return m_analysisMessage;}QString homePath()const;
 Q_INVOKABLE void analyze(QString path);Q_INVOKABLE void cancelAnalysis();
 bool busy() const { return m_busy; }
 QVariantList volumes() const { return m_volumes; }
 QString error() const { return m_error; }
 Q_INVOKABLE void refresh();
 Q_INVOKABLE bool openVolume(const QString &mount);
signals:
 void changed();void analysisChanged();
private:
 std::shared_ptr<std::atomic_bool> m_cancel;bool m_analyzing=false;QVariantList m_categories,m_largest;QString m_analysisMessage;
 bool m_busy=false;
 QVariantList m_volumes;
 QString m_error;
};
