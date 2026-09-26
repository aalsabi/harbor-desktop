#include "Translation.h"
#include "Preferences.h"
#include <QCoreApplication>

Translation::Translation(Preferences& prefs, QObject* parent) : QObject(parent), preferences(prefs) {
    if (!translator.load(":/i18n/harbor_ar.qm"))
        qWarning("Harbor: Arabic translations are missing from the build");
    apply();
    connect(&preferences, &Preferences::changed, this, &Translation::apply);
}

Translation::~Translation() {
    QCoreApplication::removeTranslator(&translator);
}

void Translation::apply() {
    const bool arabic = preferences.language() == "ar";
    if (arabic == active)
        return;
    active = arabic;
    if (arabic)
        QCoreApplication::installTranslator(&translator);
    else
        QCoreApplication::removeTranslator(&translator);
    emit changed();
}
