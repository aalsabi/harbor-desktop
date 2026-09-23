#include "Preferences.h"
#include <QStandardPaths>
#include <cmath>
Preferences::Preferences(QString path,QObject* parent):QObject(parent),settings(path.isEmpty()?QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/harbor/settings.ini":path,QSettings::IniFormat){}
double Preferences::opacity()const {auto v=settings.value("opacity",.9).toDouble();return std::isfinite(v)&&v>=.45&&v<=1?v:.9;}
bool Preferences::dark()const{return settings.value("dark",true).toBool();}
bool Preferences::reduceMotion()const{return settings.value("reduceMotion",false).toBool();}
QStringList Preferences::pins()const{return settings.value("pins",QStringList{"org.kde.dolphin.desktop","firefox-esr.desktop","org.kde.konsole.desktop","org.harbor.Settings.desktop"}).toStringList();}
void Preferences::setOpacity(double v){if(!std::isfinite(v)||v<.45||v>1)return;settings.setValue("opacity",v);settings.sync();emit changed();}
void Preferences::setDark(bool v){settings.setValue("dark",v);settings.sync();emit changed();}
void Preferences::setReduceMotion(bool v){settings.setValue("reduceMotion",v);settings.sync();emit changed();}
void Preferences::setPins(QStringList v){v.removeDuplicates();settings.setValue("pins",v);settings.sync();emit changed();}

QString Preferences::language()const{return settings.value("language","en").toString();}
void Preferences::setLanguage(QString v){if(v!="en"&&v!="ar")return;settings.setValue("language",v);settings.sync();emit changed();}
