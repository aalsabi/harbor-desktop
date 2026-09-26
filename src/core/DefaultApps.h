#pragma once
#include <QObject>
#include <QVariantList>
class DefaultApps : public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantList roles READ roles NOTIFY changed)
 Q_PROPERTY(QString message READ message NOTIFY changed)
public:
 explicit DefaultApps(QObject *parent=nullptr);
 QVariantList roles() const{return m_roles;}
 QString message() const{return m_message;}
 Q_INVOKABLE void refresh();
 Q_INVOKABLE bool setDefault(const QString &role,const QString &desktopId);
signals:void changed();
private:QVariantList m_roles;QString m_message;
};
