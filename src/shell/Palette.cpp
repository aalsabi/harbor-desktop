#include "Palette.h"
#include "core/Preferences.h"
#include <QGuiApplication>
#include <QPalette>

void applyPalette(const Preferences& preferences) {
    QPalette p;
    bool d = preferences.dark();
    p.setColor(QPalette::Window, d ? QColor("#252528") : QColor("#f6f6f8"));
    p.setColor(QPalette::WindowText, d ? QColor("#eeeeef") : QColor("#26262a"));
    p.setColor(QPalette::Text, p.color(QPalette::WindowText));
    p.setColor(QPalette::ButtonText, p.color(QPalette::WindowText));
    p.setColor(QPalette::Base, d ? QColor("#333337") : Qt::white);
    p.setColor(QPalette::Button, d ? QColor("#3d3d43") : QColor("#ededf1"));
    p.setColor(QPalette::Highlight, QColor(preferences.accent()));
    QGuiApplication::setPalette(p);
}
