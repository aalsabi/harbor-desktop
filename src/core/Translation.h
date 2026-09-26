#pragma once
#include <QObject>
#include <QTranslator>
class Preferences;
// Installs Harbor's compiled translations (translations/harbor_*.ts) for the preferred
// interface language and follows later changes. Connect `changed` to QQmlEngine::retranslate.
class Translation : public QObject {
    Q_OBJECT
public:
    explicit Translation(Preferences& preferences, QObject* parent = nullptr);
    ~Translation() override;
    bool rightToLeft() const { return active; }
signals:
    void changed();

private:
    void apply();
    Preferences& preferences;
    QTranslator translator;
    bool active = false;
};
