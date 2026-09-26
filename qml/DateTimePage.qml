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
  HarborLabel { text: qsTr("Automatic date and time"); font.bold: true }
  RowLayout {
   Layout.fillWidth: true
   Note { text: qsTr("Keep the system clock synchronized using the network.") }
   Switch { objectName: "automaticTimeSwitch"; Accessible.name: qsTr("Automatic date and time"); checked: DateTimeSettings.ntp; enabled: DateTimeSettings.available && DateTimeSettings.canNtp && !DateTimeSettings.busy; onClicked: { DateTimeSettings.setNtp(checked); checked = Qt.binding(function() { return DateTimeSettings.ntp }) } }
  }
  Note { text: DateTimeSettings.synchronized ? qsTr("Clock synchronized") : qsTr("Clock is not currently synchronized") }
 }
 Group {
  HarborLabel { text: qsTr("Time zone"); font.bold: true }
  Note { text: qsTr("Current zone: ") + (DateTimeSettings.timezone || "—") }
  HarborField { id: zoneSearch; Layout.fillWidth: true; placeholderText: qsTr("Search time zones"); Accessible.name: placeholderText }
  ComboBox { id: zone; implicitHeight: 32; leftPadding: 10; rightPadding: 28;
  background: Rectangle { radius: 6; color: Prefs.dark ? "#45454b" : "#fafafa"; border.color: zone.activeFocus ? Prefs.accent : Prefs.dark ? "#55555b" : "#dedee4" }
  contentItem: Text { text: zone.displayText; color: root.ink; font.pixelSize: 13; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
  objectName: "timezoneChoice"; Accessible.name: qsTr("Time zone"); Layout.fillWidth: true; model: DateTimeSettings.timezones.filter(function(value) { return value.toLowerCase().indexOf(zoneSearch.text.toLowerCase()) >= 0 }); currentIndex: model.indexOf(DateTimeSettings.timezone); enabled: DateTimeSettings.available && !DateTimeSettings.busy }
  HarborButton { text: qsTr("Apply time zone"); enabled: DateTimeSettings.available && !DateTimeSettings.busy && zone.currentIndex >= 0 && zone.currentText !== DateTimeSettings.timezone; onClicked: DateTimeSettings.setTimezone(zone.currentText) }
 }
 Group {
  HarborLabel { text: qsTr("Set the clock manually"); font.bold: true }
  Note { text: qsTr("Turn off automatic time first. Enter the local time in the selected time zone.") }
  RowLayout {
   Layout.fillWidth: true
   HarborField { id: date; objectName: "manualDateField"; Layout.fillWidth: true; placeholderText: "YYYY-MM-DD"; enabled: DateTimeSettings.available && !DateTimeSettings.ntp && !DateTimeSettings.busy; Accessible.name: qsTr("Date, year-month-day") }
   HarborField { id: time; objectName: "manualTimeField"; Layout.fillWidth: true; placeholderText: "HH:MM:SS"; enabled: date.enabled; Accessible.name: qsTr("Time, hours:minutes:seconds") }
  }
  HarborButton { text: qsTr("Set date and time"); enabled: date.enabled && date.text.length > 0 && time.text.length > 0; onClicked: DateTimeSettings.setDateTime(date.text, time.text) }
 }
 Note { text: qsTr("Changes may require administrator authentication.") }
 Note { visible: DateTimeSettings.error.length > 0; text: DateTimeSettings.error; color: Prefs.dark ? "#ffaaa4" : "#b22b24" }
 RowLayout {
  HarborButton { text: qsTr("Refresh"); enabled: !DateTimeSettings.busy; onClicked: DateTimeSettings.refresh() }
  Note { text: DateTimeSettings.busy ? qsTr("Updating…") : !DateTimeSettings.available ? qsTr("Date and time service unavailable") : "" }
 }
}
