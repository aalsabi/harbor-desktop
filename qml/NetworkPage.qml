pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id: root
 objectName: "networkSettingsPage"
 spacing: 16
 readonly property bool arabic: Prefs.language === "ar"
 property var secretValues: ({})
 property string selectedAp: ""
 property string editingPath: ""
 property bool editing: false
 property var draft: ({})
 function t(en, ar) { return arabic ? ar : en }
 function stage(key,value) { var next=Object.assign({},draft);next[key]=value;draft=next }
 function enterprise(ssid) { edit("");stage("enterprise",true);stage("eap","peap");stage("phase2Auth","mschapv2");stage("type","802-11-wireless");stage("ssid",ssid);stage("name",ssid) }
 function edit(path) { editingPath=path;draft=NetworkSettings.profileDraft(path);editing=true }
 LayoutMirroring.enabled: arabic
 LayoutMirroring.childrenInherit: true
 Connections { target: NetworkSettings; function onProfileSaved(path) {root.editingPath=path;root.editing=false;root.draft={}} function onChanged() {if(!NetworkSettings.secretPrompt.name)root.secretValues={}} }
 Component.onCompleted: NetworkSettings.setActive(true)
 Component.onDestruction: NetworkSettings.setActive(false)
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
   HarborLabel { Layout.fillWidth: true; text: root.t("Wi-Fi", "واي فاي"); font.bold: true }
   Switch { Accessible.name: root.t("Wi-Fi", "واي فاي"); checked: NetworkSettings.wirelessEnabled; enabled: NetworkSettings.available && !NetworkSettings.busy; onClicked: {NetworkSettings.setWirelessEnabled(checked);checked=Qt.binding(function(){return NetworkSettings.wirelessEnabled})} }
  }
  Flow {
   Layout.fillWidth: true; spacing: 8
   HarborButton { text: root.t("Scan networks", "البحث عن الشبكات"); enabled: NetworkSettings.available && NetworkSettings.wirelessEnabled && !NetworkSettings.busy; onClicked: NetworkSettings.scan() }
   HarborButton { text: root.t("Refresh", "تحديث"); enabled: !NetworkSettings.busy; onClicked: NetworkSettings.refresh() }
  }
  Note { visible: !NetworkSettings.available; text: root.t("NetworkManager is unavailable.", "خدمة NetworkManager غير متاحة.") }
 }
 Card {
  visible: !!NetworkSettings.secretPrompt.name
  HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: root.t("Authentication: ", "المصادقة: ") + (NetworkSettings.secretPrompt.name || ""); font.bold: true }
  Note { text: root.t("Enter the credentials requested for this connection attempt.", "أدخل بيانات الاعتماد المطلوبة لمحاولة الاتصال هذه.") }
  Repeater {
   model: NetworkSettings.secretPrompt.fields || []
   delegate: HarborField {
    required property var modelData
    Layout.fillWidth: true; placeholderText: modelData.label; Accessible.name: placeholderText
    echoMode: modelData.secret ? TextInput.Password : TextInput.Normal
    onTextEdited: {var next=Object.assign({},root.secretValues);next[modelData.key]=text;root.secretValues=next}
   }
  }
  RowLayout {
   HarborButton { text: root.t("Continue", "متابعة"); onClicked: {NetworkSettings.respondSecrets(true,root.secretValues);if(!NetworkSettings.secretPrompt.name)root.secretValues={}} }
   HarborButton { text: root.t("Cancel", "إلغاء"); onClicked: {NetworkSettings.cancelSecrets();root.secretValues={}} }
  }
 }
 Repeater {
  model: NetworkSettings.devices
  delegate: Card {
   id: device
   required property var modelData
   HarborLabel { text: device.modelData.interface + " · " + (device.modelData.type === "wifi" ? root.t("Wi-Fi", "واي فاي") : root.t("Wired", "سلكي")); font.bold: true }
   Note { text: Number(device.modelData.state) === 100 ? root.t("Connected", "متصل") : Number(device.modelData.state) >= 40 && Number(device.modelData.state) < 100 ? root.t("Connecting…", "جارٍ الاتصال…") : root.t("Disconnected", "غير متصل") }
   HarborButton { text: root.t("Disconnect", "قطع الاتصال"); enabled: !NetworkSettings.busy && Number(device.modelData.state) >= 40; onClicked: NetworkSettings.disconnectDevice(device.modelData.path) }
  }
 }
 HarborLabel { text: root.t("Nearby Wi-Fi networks", "شبكات واي فاي القريبة"); font.bold: true }
 Repeater {
  model: NetworkSettings.networks
  delegate: Card {
   id: ap
   required property var modelData
   HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: ap.modelData.name || root.t("Hidden network", "شبكة مخفية"); font.bold: true }
   Note { text: Number(ap.modelData.strength) + "% · " + (ap.modelData.enterprise ? root.t("Enterprise", "مؤسسية") : ap.modelData.secured ? root.t("Secured", "مؤمنة") : root.t("Open", "مفتوحة")) }
   HarborButton { text: root.t("Connect", "اتصال"); enabled: !NetworkSettings.busy; onClicked: { if(ap.modelData.enterprise)root.enterprise(ap.modelData.name);else if(ap.modelData.secured)root.selectedAp=ap.modelData.path;else NetworkSettings.connectWifi(ap.modelData.path,"") } }
   ColumnLayout {
    Layout.fillWidth: true; visible: root.selectedAp === ap.modelData.path
    Note { visible: ap.modelData.enterprise; text: root.t("Use an existing 802.1X profile from Saved connections for enterprise Wi-Fi.", "استخدم ملف 802.1X موجودًا من الاتصالات المحفوظة للشبكات المؤسسية.") }
    HarborField { id: password; Layout.fillWidth: true; visible: !ap.modelData.enterprise; placeholderText: root.t("Wi-Fi password", "كلمة مرور واي فاي"); Accessible.name: placeholderText; echoMode: TextInput.Password }
    RowLayout {
     HarborButton { visible: !ap.modelData.enterprise; text: root.t("Join network", "الانضمام إلى الشبكة"); enabled: !NetworkSettings.busy; onClicked: {NetworkSettings.connectWifi(ap.modelData.path,password.text);password.text=""} }
     HarborButton { text: root.t("Cancel", "إلغاء"); onClicked: {password.text="";root.selectedAp=""} }
    }
   }
  }
 }
 HarborLabel { text: root.t("Saved connections", "الاتصالات المحفوظة"); font.bold: true }
 Repeater {
  model: NetworkSettings.profiles
  delegate: Card {
   id: connection
   required property var modelData
   HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: connection.modelData.name; font.bold: true }
   Note { text: connection.modelData.type }
   Flow {
    Layout.fillWidth: true; spacing: 8
    HarborButton { text: root.t("Connect", "اتصال"); enabled: !NetworkSettings.busy; onClicked: NetworkSettings.activate(connection.modelData.path) }
    HarborButton { text: root.t("Edit connection", "تعديل الاتصال"); enabled: !NetworkSettings.busy; onClicked: root.edit(connection.modelData.path) }
   }
  }
 }
 HarborButton { text: root.t("New enterprise Wi-Fi", "شبكة واي فاي مؤسسية جديدة"); enabled: NetworkSettings.available && !NetworkSettings.busy; onClicked: root.enterprise("") }
 HarborButton { text: root.t("New wired connection", "اتصال سلكي جديد"); enabled: NetworkSettings.available && !NetworkSettings.busy; onClicked: root.edit("") }
 Card {
  visible: root.editing
  HarborLabel { text: root.t("Connection settings", "إعدادات الاتصال"); font.bold: true }
  HarborField { Layout.fillWidth: true; text: root.draft.name || ""; placeholderText: root.t("Connection name", "اسم الاتصال"); Accessible.name: placeholderText; onTextEdited: root.stage("name",text) }
  HarborField { Layout.fillWidth: true; visible: root.editingPath === ""; text: root.draft.interface || ""; placeholderText: root.t("Interface (optional, e.g. eth0)", "واجهة الشبكة (اختياري، مثل eth0)"); Accessible.name: placeholderText; onTextEdited: root.stage("interface",text) }
  HarborField { Layout.fillWidth: true; visible: root.editingPath === "" && root.draft.type === "802-11-wireless"; text: root.draft.ssid || ""; placeholderText: root.t("Wi-Fi network name (SSID)", "اسم شبكة واي فاي (SSID)"); Accessible.name: placeholderText; onTextEdited: root.stage("ssid",text) }
  CheckBox { visible: root.draft.type === "802-3-ethernet" && !root.draft.enterprise; text: root.t("Configure 802.1X authentication", "إعداد مصادقة 802.1X"); onClicked: {root.stage("enterprise",true);root.stage("eap","peap");root.stage("phase2Auth","mschapv2")} }
  ColumnLayout {
   visible: !!root.draft.enterprise; Layout.fillWidth: true
   HarborLabel { text: root.t("Enterprise authentication · 802.1X", "مصادقة المؤسسات · 802.1X"); font.bold: true }
   Note { visible: root.draft.enterpriseEditable === false; text: root.t("This profile uses advanced authentication managed outside Harbor. Its authentication settings are preserved.", "يستخدم هذا الملف مصادقة متقدمة تُدار خارج هاربور. يتم الاحتفاظ بإعداداتها.") }
   ColumnLayout {
    visible: root.draft.enterpriseEditable !== false; Layout.fillWidth: true
    ComboBox { Layout.fillWidth: true; model: ["PEAP","TTLS","TLS"]; currentIndex: ["peap","ttls","tls"].indexOf(root.draft.eap || "peap"); Accessible.name: root.t("EAP authentication", "مصادقة EAP"); onActivated: {root.stage("eap",["peap","ttls","tls"][currentIndex]);root.stage("phase2Auth","mschapv2")} }
    Repeater {
     model: [{key:"identity",en:"Identity",ar:"الهوية"},{key:"anonymousIdentity",en:"Anonymous identity (optional)",ar:"الهوية المجهولة (اختياري)"},{key:"domainSuffixMatch",en:"Authentication server domain",ar:"نطاق خادم المصادقة"},{key:"caCert",en:"CA certificate file path",ar:"مسار ملف شهادة جهة التصديق"}]
     delegate: HarborField { required property var modelData; Layout.fillWidth: true; text: root.draft[modelData.key] || ""; placeholderText: root.t(modelData.en,modelData.ar); Accessible.name: placeholderText; onTextEdited: root.stage(modelData.key,text) }
    }
    ComboBox { visible: root.draft.eap !== "tls"; Layout.fillWidth: true; model: root.draft.eap === "ttls" ? ["mschapv2","pap","chap","mschap"] : ["mschapv2","gtc"]; currentIndex: Math.max(0,model.indexOf(root.draft.phase2Auth || "mschapv2")); Accessible.name: root.t("Inner authentication", "المصادقة الداخلية"); onActivated: root.stage("phase2Auth",model[currentIndex]) }
    HarborField { visible: root.draft.eap !== "tls"; Layout.fillWidth: true; text: root.draft.eapPassword || ""; placeholderText: root.t("Password (blank keeps saved password or asks on connect)", "كلمة المرور (فارغة للاحتفاظ بالحالية أو طلبها عند الاتصال)"); Accessible.name: placeholderText; echoMode: TextInput.Password; onTextEdited: root.stage("eapPassword",text) }
    Repeater {
     model: [{key:"clientCert",en:"Client certificate file path",ar:"مسار شهادة العميل"},{key:"privateKey",en:"Private key file path",ar:"مسار المفتاح الخاص"}]
     delegate: HarborField { required property var modelData; visible: root.draft.eap === "tls"; Layout.fillWidth: true; text: root.draft[modelData.key] || ""; placeholderText: root.t(modelData.en,modelData.ar); Accessible.name: placeholderText; onTextEdited: root.stage(modelData.key,text) }
    }
    HarborField { visible: root.draft.eap === "tls"; Layout.fillWidth: true; text: root.draft.privateKeyPassword || ""; placeholderText: root.t("Private key password (blank to keep)", "كلمة مرور المفتاح الخاص (فارغة للاحتفاظ بالحالية)"); Accessible.name: placeholderText; echoMode: TextInput.Password; onTextEdited: root.stage("privateKeyPassword",text) }
    Note { text: root.t("Use the CA certificate and server domain provided by your administrator. Certificate verification is required. Keep certificate files in a permanent location. New credentials are saved by NetworkManager; new profiles are restricted to your user.", "استخدم شهادة جهة التصديق ونطاق الخادم من مسؤول الشبكة. التحقق من الشهادة مطلوب. احتفظ بالملفات في موقع دائم. يحفظ NetworkManager بيانات الاعتماد وتقتصر الملفات الجديدة على المستخدم الحالي.") }
   }
  }
  Repeater {
   model: ["ipv4", "ipv6"]
   delegate: ColumnLayout {
    id: ip
    required property string modelData
    Layout.fillWidth: true; spacing: 8
    HarborLabel { text: ip.modelData.toUpperCase(); font.bold: true }
    ComboBox { Layout.fillWidth: true; model: [root.t("Automatic", "تلقائي"),root.t("Manual", "يدوي"),root.t("Disabled", "معطل")]; currentIndex: ["auto","manual","disabled"].indexOf(root.draft[ip.modelData+"Method"] || "auto"); Accessible.name: ip.modelData + root.t(" addressing", " العنونة"); onActivated: root.stage(ip.modelData+"Method",["auto","manual","disabled"][currentIndex]) }
    HarborField { Layout.fillWidth: true; visible: root.draft[ip.modelData+"Method"] === "manual"; text: root.draft[ip.modelData+"Addresses"] || ""; placeholderText: ip.modelData === "ipv4" ? "192.168.1.10/24" : "2001:db8::10/64"; Accessible.name: ip.modelData + root.t(" addresses and prefixes", " العناوين والبادئات"); onTextEdited: root.stage(ip.modelData+"Addresses",text) }
    HarborField { Layout.fillWidth: true; visible: root.draft[ip.modelData+"Method"] === "manual"; text: root.draft[ip.modelData+"Gateway"] || ""; placeholderText: root.t("Gateway (optional)", "البوابة (اختياري)"); Accessible.name: ip.modelData + placeholderText; onTextEdited: root.stage(ip.modelData+"Gateway",text) }
    CheckBox { text: root.t("Use automatic DNS", "استخدام DNS التلقائي"); checked: !root.draft[ip.modelData+"ManualDns"]; enabled: root.draft[ip.modelData+"Method"] !== "disabled"; onClicked: root.stage(ip.modelData+"ManualDns",!checked) }
    HarborField { Layout.fillWidth: true; enabled: root.draft[ip.modelData+"Method"] !== "disabled"; text: root.draft[ip.modelData+"Dns"] || ""; placeholderText: root.t("DNS servers, separated by commas", "خوادم DNS مفصولة بفواصل"); Accessible.name: ip.modelData + placeholderText; onTextEdited: root.stage(ip.modelData+"Dns",text) }
   }
  }
  ColumnLayout {
   visible: !!root.draft.vpnOpenvpn; Layout.fillWidth: true
   HarborLabel { text: root.t("VPN sign-in", "تسجيل الدخول إلى VPN"); font.bold: true }
   HarborField { Layout.fillWidth: true; text: root.draft.vpnUsername || ""; placeholderText: root.t("VPN username", "اسم مستخدم VPN"); Accessible.name: placeholderText; onTextEdited: root.stage("vpnUsername",text) }
   HarborField { id: vpnPassword; Layout.fillWidth: true; placeholderText: root.t("New password (leave blank to keep)", "كلمة مرور جديدة (اتركها فارغة للاحتفاظ بالحالية)"); Accessible.name: placeholderText; echoMode: TextInput.Password; onTextEdited: root.stage("vpnPassword",text) }
   Note { text: root.t("A new password is saved by NetworkManager with this connection. Imported VPNs are restricted to your user.", "يحفظ NetworkManager كلمة المرور الجديدة مع هذا الاتصال. ملفات VPN المستوردة مقيدة بالمستخدم الحالي.") }
  }
  Note { text: root.t("Saving changes the profile. Connect again to apply it to the active network.", "الحفظ يغيّر ملف الاتصال. أعد الاتصال لتطبيقه على الشبكة النشطة.") }
  RowLayout {
   HarborButton { text: root.t("Save profile", "حفظ ملف الاتصال"); enabled: !NetworkSettings.busy; onClicked: {NetworkSettings.saveProfile(root.editingPath,root.draft);root.stage("vpnPassword","");root.stage("eapPassword","");root.stage("privateKeyPassword","");vpnPassword.text=""} }
   HarborButton { text: root.t("Close", "إغلاق"); onClicked: {root.editing=false;root.draft={}} }
  }
 }
 Card {
  HarborLabel { text: root.t("Import VPN", "استيراد VPN"); font.bold: true }
  Note { text: root.t("Import a local OpenVPN or WireGuard configuration. The matching NetworkManager plugin must be installed. Imported profiles are restricted to your user.", "استورد إعداد OpenVPN أو WireGuard محليًا. يجب تثبيت إضافة NetworkManager المناسبة. يقتصر ملف الاتصال المستورد على المستخدم الحالي.") }
  ComboBox { id: vpnType; Layout.fillWidth: true; model: ["OpenVPN","WireGuard"]; Accessible.name: root.t("VPN type", "نوع VPN") }
  HarborField { id: vpnFile; Layout.fillWidth: true; placeholderText: root.t("Configuration file path", "مسار ملف الإعداد"); Accessible.name: placeholderText }
  HarborButton { text: root.t("Import", "استيراد"); enabled: NetworkSettings.available && !NetworkSettings.busy && vpnFile.text.length > 0; onClicked: NetworkSettings.importVpn(vpnFile.text,vpnType.currentIndex === 0 ? "openvpn" : "wireguard") }
 }
 Note { visible: NetworkSettings.busy; text: root.t("Updating network settings…", "جارٍ تحديث إعدادات الشبكة…") }
 Note { visible: NetworkSettings.error.length > 0; text: NetworkSettings.error; color: Prefs.dark ? "#ffaaa4" : "#b22b24" }
}
