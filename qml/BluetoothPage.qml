pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id: root
 objectName: "bluetoothSettingsPage"
 readonly property bool arabic: Prefs.language === "ar"
 readonly property var state: System.state
 readonly property var prompt: state.bluetoothPrompt || ({})
 property string forgetPath: ""
 function t(en, ar) { return arabic ? ar : en }
 LayoutMirroring.enabled: arabic
 LayoutMirroring.childrenInherit: true
 spacing: 16
 component Note: HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Prefs.dark ? "#aaaab2" : "#696971" }
 component Card: Rectangle {
  default property alias contents: layout.data
  Layout.fillWidth: true; implicitHeight: layout.implicitHeight + 28
  radius: 12; color: Prefs.dark ? "#29292f" : "#ffffff"; border.color: Prefs.dark ? "#45454d" : "#dedee4"
  ColumnLayout { id: layout; anchors { left: parent.left; right: parent.right; top: parent.top; margins: 14 } spacing: 10 }
 }
 Card {
  RowLayout {
   Layout.fillWidth: true
   HarborLabel { Layout.fillWidth: true; text: "Bluetooth"; font.bold: true }
   Switch { Accessible.name: "Bluetooth"; checked: !!root.state.bluetoothPowered; enabled: !!root.state.bluetoothAvailable && !root.state.bluetoothBusy; onClicked: { System.action("bluetooth",checked); checked=Qt.binding(function(){return !!root.state.bluetoothPowered}) } }
  }
  HarborButton { text: root.t("Search for devices", "البحث عن أجهزة"); enabled: !!root.state.bluetoothPowered && !root.state.bluetoothBusy; onClicked: System.action("bluetooth-scan") }
  Note { visible: !root.state.bluetoothAvailable; text: root.t("Bluetooth service or adapter unavailable.", "خدمة بلوتوث أو المحول غير متاح.") }
 }
 Card {
  visible: !!root.prompt.kind
  HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; font.bold: true; text: root.t("Pair with ", "الاقتران مع ") + (root.prompt.name || "") }
  Note { text: root.prompt.kind === "confirm" ? root.t("Confirm that this code is shown on the other device.", "تأكد من ظهور هذا الرمز على الجهاز الآخر.") : root.prompt.kind === "display" ? root.t("Enter this code on the other device.", "أدخل هذا الرمز على الجهاز الآخر.") : root.prompt.kind === "authorize" ? root.t("Allow this device to pair or access the requested service?", "هل تسمح لهذا الجهاز بالاقتران أو الوصول إلى الخدمة المطلوبة؟") : root.t("Enter the PIN or passkey shown by the device.", "أدخل رمز PIN أو مفتاح المرور الذي يعرضه الجهاز.") }
  HarborLabel { visible: !!root.prompt.code; text: root.prompt.code || ""; font.pixelSize: 24; font.bold: true; Layout.fillWidth: true; wrapMode: Text.WrapAnywhere }
  HarborField { id: pin; Layout.fillWidth: true; visible: root.prompt.kind === "pin" || root.prompt.kind === "passkey"; echoMode: TextInput.Password; placeholderText: root.t("PIN / passkey", "رمز PIN / مفتاح المرور"); Accessible.name: placeholderText; onVisibleChanged: text="" }
  RowLayout {
   HarborButton { visible: root.prompt.kind !== "display"; text: root.t("Accept", "قبول"); onClicked: { System.action("bluetooth-respond",{accept:true,value:pin.text}); pin.text="" } }
   HarborButton { text: root.t("Cancel", "إلغاء"); onClicked: {System.action("bluetooth-cancel-pair");pin.text=""} }
  }
 }
 Repeater {
  model: root.state.bluetoothDevices || []
  delegate: Card {
   id: device
   required property var modelData
   HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: device.modelData.name; font.bold: true }
   Note { text: device.modelData.connected ? root.t("Connected", "متصل") : device.modelData.paired ? root.t("Paired", "مقترن") : root.t("Not paired", "غير مقترن") }
   Flow {
    Layout.fillWidth: true; spacing: 8
    HarborButton { text: !device.modelData.paired ? root.t("Pair", "اقتران") : device.modelData.connected ? root.t("Disconnect", "قطع الاتصال") : root.t("Connect", "اتصال"); enabled: !root.state.bluetoothBusy && !!root.state.bluetoothPowered; onClicked: System.action(!device.modelData.paired ? "bluetooth-pair" : device.modelData.connected ? "bluetooth-disconnect" : "bluetooth-connect", !device.modelData.paired ? device.modelData.path : device.modelData.address) }
    HarborButton { visible: device.modelData.paired; text: root.t("Forget device", "نسيان الجهاز"); enabled: !root.state.bluetoothBusy; onClicked: root.forgetPath=device.modelData.path }
   }
   ColumnLayout {
    visible: root.forgetPath === device.modelData.path; Layout.fillWidth: true
    Note { text: root.t("Remove this pairing? You will need to pair again to reconnect.", "هل تريد إزالة الاقتران؟ ستحتاج إلى الاقتران مجددًا للاتصال.") }
    RowLayout {
     HarborButton { text: root.t("Forget", "نسيان"); onClicked: {System.action("bluetooth-unpair",root.forgetPath);root.forgetPath=""} }
     HarborButton { text: root.t("Cancel", "إلغاء"); onClicked: root.forgetPath="" }
    }
   }
  }
 }
 Note { visible: !!root.state.bluetoothError; text: root.state.bluetoothError || "" }
 Note { visible: !!root.state.bluetoothBusy; text: root.t("Waiting for the Bluetooth device…", "بانتظار جهاز بلوتوث…") }
 HarborButton { visible: !!root.state.bluetoothBusy; text: root.t("Cancel pairing", "إلغاء الاقتران"); onClicked: System.action("bluetooth-cancel-pair") }
 Component.onDestruction: {System.action("bluetooth-stop-scan");System.action("bluetooth-cancel-pair")}
}
