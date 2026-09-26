#pragma once
#include <QObject>
#include <functional>
class Controller : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.harbor.Shell")
public:
    using QObject::QObject;
    std::function<void(QString)> onOpen;
    std::function<void()> onClose, onLogout;
    std::function<QString()> onList;
    std::function<void(QString)> onActivate, onWindowClose;
    std::function<void(QString)> onWindowAction;
    Q_INVOKABLE void open(QString page) {
        if (onOpen)
            onOpen(page);
    }
    Q_INVOKABLE void windowAction(QString name) {
        if (onWindowAction)
            onWindowAction(name);
    }
    Q_INVOKABLE void logout() {
        if (onLogout)
            onLogout();
    }
    Q_INVOKABLE void dismiss() {
        if (onClose)
            onClose();
    }
public slots:
    void Logout() { logout(); }
    void Show(QString page) { open(page); }
    QString WindowList() { return onList ? onList() : QString("[]"); }
    void Activate(QString id) {
        if (onActivate)
            onActivate(id);
    }
    void Close(QString id) {
        if (onWindowClose)
            onWindowClose(id);
    }
};
