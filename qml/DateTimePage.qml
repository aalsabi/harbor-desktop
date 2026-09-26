pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id: root
 objectName: "dateTimePage"
 spacing: 16
 readonly property bool arabic: Prefs.language === "ar"
 readonly property color ink: Prefs.dark ? "#eeeeef" : "#26262a"
 readonly property color muted: Prefs.dark ? "#aaaab2" : "#696971"
 function t(en, ar) { return arabic ? ar : en }
 LayoutMirroring.enabled: arabic
 LayoutMirroring.childrenInherit: true
 Component.onCompleted: DateTimeSettings.setActive(true)
 Component.onDestruction: DateTimeSettings.setActive(false)
 component Note: HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: root.muted; font.pixelSize: 12 }
 component Group: Rectangle {
  default property alias items: contents.data
  Layout.fillWidth: true
  implicitHeight: contents.implicitHeight + 32
  radius: 12; color: Prefs.dark ? "#29292f" : "#ffffff"
  border.color: Prefs.dark ? "#45454d" : "#dedee4"
  ColumnLayout { id: contents; anchors { left: parent.left; right: parent.right; top: parent.top; margins: 16 } spacing: 12 }
 }
 HarborLabel { Layout.fillWidth: true; text: DateTimeSettings.currentDateTime || "—"; font.pixelSize: 24; font.bold: true; horizontalAlignment: Text.AlignHCenter }
 Group {
  HarborLabel { text: root.t("Automatic date and time", "التاريخ والوقت التلقائيان"); font.bold: true }
  RowLayout {
   Layout.fillWidth: true
   Note { text: root.t("Keep the system clock synchronized using the network.", "مزامنة ساعة النظام باستخدام الشبكة.") }
   Switch { objectName: "automaticTimeSwitch"; Accessible.name: root.t("Automatic date and time", "التاريخ والوقت التلقائيان"); checked: DateTimeSettings.ntp; enabled: DateTimeSettings.available && DateTimeSettings.canNtp && !DateTimeSettings.busy; onClicked: { DateTimeSettings.setNtp(checked); checked = Qt.binding(function() { return DateTimeSettings.ntp }) } }
  }
  Note { text: DateTimeSettings.synchronized ? root.t("Clock synchronized", "تمت مزامنة الساعة") : root.t("Clock is not currently synchronized", "الساعة غير متزامنة حاليًا") }
 }
 Group {
  HarborLabel { text: root.t("Time zone", "المنطقة الزمنية"); font.bold: true }
  Note { text: root.t("Current zone: ", "المنطقة الحالية: ") + (DateTimeSettings.timezone || "—") }
  HarborField { id: zoneSearch; Layout.fillWidth: true; placeholderText: root.t("Search time zones", "البحث في المناطق الزمنية"); Accessible.name: placeholderText }
  ComboBox { id: zone; implicitHeight: 32; leftPadding: 10; rightPadding: 28;
  background: Rectangle { radius: 6; color: Prefs.dark ? "#45454b" : "#fafafa"; border.color: zone.activeFocus ? Prefs.accent : Prefs.dark ? "#55555b" : "#dedee4" }
  contentItem: Text { text: zone.displayText; color: root.ink; font.pixelSize: 13; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
  objectName: "timezoneChoice"; Accessible.name: root.t("Time zone", "المنطقة الزمنية"); Layout.fillWidth: true; model: DateTimeSettings.timezones.filter(function(value) { return value.toLowerCase().indexOf(zoneSearch.text.toLowerCase()) >= 0 }); currentIndex: model.indexOf(DateTimeSettings.timezone); enabled: DateTimeSettings.available && !DateTimeSettings.busy }
  HarborButton { text: root.t("Apply time zone", "تطبيق المنطقة الزمنية"); enabled: DateTimeSettings.available && !DateTimeSettings.busy && zone.currentIndex >= 0 && zone.currentText !== DateTimeSettings.timezone; onClicked: DateTimeSettings.setTimezone(zone.currentText) }
 }
 Group {
  HarborLabel { text: root.t("Set the clock manually", "ضبط الساعة يدويًا"); font.bold: true }
  Note { text: root.t("Turn off automatic time first. Enter the local time in the selected time zone.", "أوقف الوقت التلقائي أولاً. أدخل الوقت المحلي للمنطقة الزمنية المحددة.") }
  RowLayout {
   Layout.fillWidth: true
   HarborField { id: date; objectName: "manualDateField"; Layout.fillWidth: true; placeholderText: "YYYY-MM-DD"; enabled: DateTimeSettings.available && !DateTimeSettings.ntp && !DateTimeSettings.busy; Accessible.name: root.t("Date, year-month-day", "التاريخ، سنة-شهر-يوم") }
   HarborField { id: time; objectName: "manualTimeField"; Layout.fillWidth: true; placeholderText: "HH:MM:SS"; enabled: date.enabled; Accessible.name: root.t("Time, hours:minutes:seconds", "الوقت، ساعة:دقيقة:ثانية") }
  }
  HarborButton { text: root.t("Set date and time", "ضبط التاريخ والوقت"); enabled: date.enabled && date.text.length > 0 && time.text.length > 0; onClicked: DateTimeSettings.setDateTime(date.text, time.text) }
 }
 Note { text: root.t("Changes may require administrator authentication.", "قد تتطلب التغييرات مصادقة المسؤول.") }
 Note { visible: DateTimeSettings.error.length > 0; text: DateTimeSettings.error; color: Prefs.dark ? "#ffaaa4" : "#b22b24" }
 RowLayout {
  HarborButton { text: root.t("Refresh", "تحديث"); enabled: !DateTimeSettings.busy; onClicked: DateTimeSettings.refresh() }
  Note { text: DateTimeSettings.busy ? root.t("Updating…", "جارٍ التحديث…") : !DateTimeSettings.available ? root.t("Date and time service unavailable", "خدمة التاريخ والوقت غير متاحة") : "" }
 }
}
