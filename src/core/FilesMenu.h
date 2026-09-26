#pragma once
#include "GlobalMenu.h"
class FilesMenu : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.canonical.dbusmenu")
public:
    explicit FilesMenu(QObject* parent = nullptr);
    ~FilesMenu() override;
    QString objectPath() const { return address; }
    Q_INVOKABLE void setState(QVariantMap state);
public slots:
    uint GetLayout(int parentId, int depth, QStringList properties, MenuLayout& tree);
    bool AboutToShow(int) { return false; }
    void Event(int id, QString event, QDBusVariant data, uint timestamp);
signals:
    void action(QString action);
    void LayoutUpdated(uint revision, int parent);

private:
    QString address;
    QVariantMap state;
    uint revision = 1;
    MenuLayout layout() const;
};
