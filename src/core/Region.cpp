#include "Region.h"
#include <QCalendar>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QSaveFile>
#include <QProcess>
#include <QSet>
#include <QStandardPaths>
#include <algorithm>

namespace {
QStringList strings(const QVariant &v) {
    QStringList result;
    for (const auto &s : v.toList()) result << s.toString();
    if (result.isEmpty()) result = v.toStringList();
    return result;
}
QLocale formatLocale(const QVariantMap &s) {
    const QLocale region(s.value("region").toString());
    const QLocale language(s.value("formatLanguage").toString());
    return QLocale(language.language(), region.territory());
}
QString numeric(const QLocale &locale, double value, QString group, QString decimal) {
    QString out = locale.toString(value, 'f', 2);
    out.replace(locale.groupSeparator(), QChar(0xe000));
    out.replace(locale.decimalPoint(), QChar(0xe001));
    out.replace(QChar(0xe000), group); out.replace(QChar(0xe001), decimal);
    return out;
}
QString period(QString value, const QLocale &locale, const QVariantMap &s) {
    value.replace(locale.amText(), s.value("am").toString());
    value.replace(locale.pmText(), s.value("pm").toString());
    return value;
}
}
Region::Region(QString root, QObject *parent) : QObject(parent) {
    if (root.isEmpty()) root = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    m_path = root + "/harbor/region.json";
    QProcess process; process.start("/usr/bin/locale", {"-a"});
    if(process.waitForFinished(1500)){
        for(auto name:QString::fromUtf8(process.readAllStandardOutput()).split('\n')) { name=name.trimmed().toLower(); name.remove('-'); m_availableLocales << name; }
    } else { process.kill(); process.waitForFinished(200); }
    QSet<QString> seenLanguages, seenRegions, seenCurrencies;
    for (const auto &locale : QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyTerritory)) {
        if (locale.language() == QLocale::C) continue;
        const auto currency = locale.currencySymbol(QLocale::CurrencyIsoCode);
        if (!currency.isEmpty() && !seenCurrencies.contains(currency)) {
            seenCurrencies.insert(currency);
            m_currencies << QVariantMap{{"code",currency},{"name",currency + " — " + locale.currencySymbol(QLocale::CurrencySymbol)}};
        }
        const auto lang = QLocale::languageToCode(locale.language());
        if (!seenLanguages.contains(lang)) {
            seenLanguages.insert(lang);
            m_languages << QVariantMap{{"code",lang},{"name",QLocale::languageToString(locale.language())},{"nativeName",locale.nativeLanguageName()}};
        }
        if (!seenRegions.contains(locale.name())) {
            seenRegions.insert(locale.name());
            m_regions << QVariantMap{{"code",locale.name()},{"name",QLocale::territoryToString(locale.territory()) + " — " + QLocale::languageToString(locale.language())}};
        }
    }
    auto sort = [](QVariantList &list) { std::sort(list.begin(), list.end(), [](const QVariant &a,const QVariant &b){return a.toMap().value("name").toString() < b.toMap().value("name").toString();}); };
    sort(m_languages); sort(m_regions); sort(m_currencies);
    for (const auto &name : QCalendar::availableCalendars())
        m_calendars << QVariantMap{{"code",name},{"name",name}};
    auto system = QLocale::system();
    m_state = defaults(system.language() == QLocale::C ? "en_US" : system.name());
    connect(&m_watcher,&QFileSystemWatcher::fileChanged,this,[this]{reload();watch();});
    connect(&m_watcher,&QFileSystemWatcher::directoryChanged,this,[this]{reload();watch();});
    QDir().mkpath(QFileInfo(m_path).absolutePath());
    reload(); watch();
}
QVariantMap Region::defaults(QString code) const {
    QLocale l(code);
    if (l.language() == QLocale::C) l = QLocale("en_US");
    const bool h24 = !l.timeFormat(QLocale::ShortFormat).contains("AP",Qt::CaseInsensitive);
    const QString shortTime = h24 ? "HH:mm" : "h:mm AP";
    const QString longTime = h24 ? "HH:mm:ss" : "h:mm:ss AP";
    return {{"schema",1},{"languages",QStringList{QLocale::languageToCode(l.language())}},
        {"region",l.name()},{"firstDay",int(l.firstDayOfWeek())},{"calendar","Gregorian"},{"hour24",h24},
        {"formatLanguage",QLocale::languageToCode(l.language())},{"numberGroup",l.groupSeparator()},{"numberDecimal",l.decimalPoint()},
        {"currency",l.currencySymbol(QLocale::CurrencyIsoCode).isEmpty() ? QString("USD") : l.currencySymbol(QLocale::CurrencyIsoCode)},{"currencyGroup",l.groupSeparator()},{"currencyDecimal",l.decimalPoint()},
        {"measurement",l.measurementSystem()==QLocale::ImperialUSSystem ? "us" : l.measurementSystem()==QLocale::ImperialUKSystem ? "uk" : "metric"},
        {"dateFormats",QStringList{l.dateFormat(QLocale::ShortFormat),"d MMM yyyy","d MMMM yyyy",l.dateFormat(QLocale::LongFormat)}},
        {"timeFormats",QStringList{shortTime,longTime,longTime+" t",longTime+" tttt"}},{"am",l.amText()},{"pm",l.pmText()}};
}
QString Region::validate(const QVariantMap &s) const {
    auto has = [](const QVariantList &list,const QString &code){for(const auto &v:list) if(v.toMap().value("code").toString()==code)return true;return false;};
    if (!has(m_regions,s.value("region").toString())) return "Choose a supported region.";
    const auto langs = strings(s.value("languages"));
    if(langs.isEmpty() || langs.size()>32) return "Choose at least one preferred language.";
    QSet<QString> seen;
    for(const auto &l:langs){ if(!has(m_languages,l)||seen.contains(l))return "Preferred languages must be supported and unique."; seen.insert(l); }
    if(!has(m_languages,s.value("formatLanguage").toString()))return "Choose a supported format language.";
    if(!has(m_calendars,s.value("calendar").toString()) || !QCalendar(s.value("calendar").toString()).isValid())return "Choose a supported calendar.";
    if(s.value("firstDay").toInt()<1 || s.value("firstDay").toInt()>7)return "Choose a valid first day of the week.";
    if(!QStringList{"metric","us","uk"}.contains(s.value("measurement").toString()))return "Choose a supported measurement system.";
    for(const auto &prefix: {QString("number"),QString("currency")}) {
        auto group=s.value(prefix+"Group").toString(), decimal=s.value(prefix+"Decimal").toString();
        if(group.size()>4 || decimal.isEmpty() || decimal.size()>4 || group==decimal)return "Decimal and grouping separators must differ and use at most four characters.";
        const QLocale regional(s.value("region").toString());
        for(const auto &part:{group,decimal}) {
            if(part==regional.groupSeparator()||part==regional.decimalPoint())continue;
            for(const auto c:part)if(c.isDigit()||c.isLetter()||!c.isPrint())return "Separators must be printable punctuation or spaces.";
        }
    }
    auto currency=s.value("currency").toString();
    if(currency.isEmpty()||!has(m_currencies,currency))return "Choose a valid ISO currency code.";
    for(const auto &key:{"dateFormats","timeFormats"}){
        auto patterns=strings(s.value(key)); if(patterns.size()!=4)return "Provide four date and four time formats.";
        for(const auto &p:patterns){if(p.trimmed().isEmpty()||p.size()>128)return "Format patterns must contain 1–128 characters.";for(auto c:p)if(c.category()==QChar::Other_Control)return "Format patterns must be printable."; if(p.count('\'')%2)return "Close quoted text in format patterns.";}
    }
    for(const auto &key:{"am","pm"}){auto value=s.value(key).toString();if(value.isEmpty()||value.size()>32)return "AM and PM labels must contain 1–32 characters.";for(auto c:value)if(c.category()==QChar::Other_Control)return "AM and PM labels must be printable.";}
    return {};
}
QVariantMap Region::preview(QVariantMap s) const {
    auto error=validate(s); if(!error.isEmpty())return {{"error",error}};
    auto l=formatLocale(s); QCalendar calendar(s.value("calendar").toString());
    QStringList dates,times;
    const QDate date(2026,9,23); const QTime time(17,8,9);
    for(const auto &p:strings(s.value("dateFormats")))dates<<l.toString(date,p,calendar);
    for(const auto &p:strings(s.value("timeFormats")))times<<period(l.toString(time,p),l,s);
    QStringList weekdays; for(int i=0;i<7;++i) weekdays << l.dayName((s.value("firstDay").toInt()-1+i)%7+1);
    QString measurementExample=s.value("measurement")=="metric" ? "1 km · 1 L · 20 °C" : s.value("measurement")=="us" ? "1 mile · 1 US gal · 68 °F" : "1 mile · 1 UK gal · 20 °C";
    return {{"weekdays",weekdays},{"measurementExample",measurementExample},{"date",dates.first()},{"time",times.first()},{"dates",dates},{"times",times},
        {"number",numeric(l,1234567.89,s.value("numberGroup").toString(),s.value("numberDecimal").toString())},
        {"currency",s.value("currency").toString()+" "+numeric(l,1234567.89,s.value("currencyGroup").toString(),s.value("currencyDecimal").toString())}};
}
bool Region::apply(QVariantMap draft) {
    m_message=validate(draft); if(!m_message.isEmpty()){emit changed();return false;}
    draft.insert("schema",1);
    QSaveFile file(m_path);
    auto data=QJsonDocument(QJsonObject::fromVariantMap(draft)).toJson();
    if(!QDir().mkpath(QFileInfo(m_path).absolutePath())||!file.open(QIODevice::WriteOnly)||file.write(data)!=data.size()||!file.commit()){
        m_message="Could not save regional preferences. Your previous settings remain active.";emit changed();return false;
    }
    m_state=draft; m_message="Regional preferences saved. Custom formats apply to Harbor previews and the panel clock. Standard locale settings apply to other applications after signing out and back in.";
    if(!localeNotice().isEmpty()) m_message += " " + localeNotice();
    watch();emit changed();return true;
}
void Region::reload(){
    QFile file(m_path);if(!file.open(QIODevice::ReadOnly)||file.size()>65536)return;
    QJsonParseError error; auto document=QJsonDocument::fromJson(file.readAll(),&error);
    if(error.error!=QJsonParseError::NoError||!document.isObject())return;
    auto next=document.object().toVariantMap();if(next.value("schema").toInt()!=1||!validate(next).isEmpty())return;
    if(next!=m_state){m_state=next;emit changed();}
}
void Region::watch(){
    auto dir=QFileInfo(m_path).absolutePath();
    if(QFileInfo(dir).isDir()&&!m_watcher.directories().contains(dir))m_watcher.addPath(dir);
    if(QFile::exists(m_path)&&!m_watcher.files().contains(m_path))m_watcher.addPath(m_path);
}
QString Region::clockText() const {
    const auto now=QDateTime::currentDateTime();const auto l=formatLocale(m_state);
    return l.toString(now.date(),strings(m_state.value("dateFormats")).first(),QCalendar(m_state.value("calendar").toString()))+"  "+period(l.toString(now.time(),strings(m_state.value("timeFormats")).first()),l,m_state);
}

QString Region::localeNotice() const { return availabilityNotice(m_state,m_availableLocales); }
QString Region::availabilityNotice(const QVariantMap &state,const QStringList &availableLocales) {
    const QString code=state.value("region").toString().toLower();
    QStringList notices;
    if(!availableLocales.contains(code+".utf8"))
        notices << "The selected region's UTF-8 locale is not generated on this computer. Other applications use a fallback for regional formats.";
    const auto preferred=strings(state.value("languages"));
    if(!preferred.isEmpty()) {
        const QString language=preferred.first().toLower();
        bool available=false;
        for(const auto &locale:availableLocales)
            if(locale.endsWith(".utf8") && (locale.startsWith(language+"_")||locale==language+".utf8")){available=true;break;}
        if(!available) notices << "The primary language's UTF-8 locale is not generated. Other applications may use a fallback language.";
    }
    const auto measurement=state.value("measurement").toString();
    bool measurementAvailable=false;
    if(measurement=="us"||measurement=="uk")
        measurementAvailable=availableLocales.contains(measurement=="us"?"en_us.utf8":"en_gb.utf8");
    else for(const auto &locale:availableLocales){
        const auto territory=locale.section('.',0,0).section('_',-1);
        if(locale.contains('_')&&locale.endsWith(".utf8")&&!QStringList{"us","gb","lr","mm"}.contains(territory)){measurementAvailable=true;break;}
    }
    if(!measurementAvailable)notices << "A UTF-8 locale for the selected measurement system is not generated. Other applications use regional measurement defaults.";
    if(!notices.isEmpty())notices << "Harbor keeps your preferences. See the Language & Region guide to generate locales.";
    return notices.join(' ');
}
