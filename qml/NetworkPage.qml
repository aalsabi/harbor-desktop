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
   HarborLabel { Layout.fillWidth: true; text: qsTr("Wi-Fi"); font.bold: true }
   Switch { Accessible.name: qsTr("Wi-Fi"); checked: NetworkSettings.wirelessEnabled; enabled: NetworkSettings.available && !NetworkSettings.busy; onClicked: {NetworkSettings.setWirelessEnabled(checked);checked=Qt.binding(function(){return NetworkSettings.wirelessEnabled})} }
  }
  Flow {
   Layout.fillWidth: true; spacing: 8
   HarborButton { text: qsTr("Scan networks"); enabled: NetworkSettings.available && NetworkSettings.wirelessEnabled && !NetworkSettings.busy; onClicked: NetworkSettings.scan() }
   HarborButton { text: qsTr("Refresh"); enabled: !NetworkSettings.busy; onClicked: NetworkSettings.refresh() }
  }
  Note { visible: !NetworkSettings.available; text: qsTr("NetworkManager is unavailable.") }
 }
 Card {
  visible: !!NetworkSettings.secretPrompt.name
  HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: qsTr("Authentication: ") + (NetworkSettings.secretPrompt.name || ""); font.bold: true }
  Note { text: qsTr("Enter the credentials requested for this connection attempt.") }
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
   HarborButton { text: qsTr("Continue"); onClicked: {NetworkSettings.respondSecrets(true,root.secretValues);if(!NetworkSettings.secretPrompt.name)root.secretValues={}} }
   HarborButton { text: qsTr("Cancel"); onClicked: {NetworkSettings.cancelSecrets();root.secretValues={}} }
  }
 }
 Repeater {
  model: NetworkSettings.devices
  delegate: Card {
   id: device
   required property var modelData
   HarborLabel { text: device.modelData.interface + " · " + (device.modelData.type === "wifi" ? qsTr("Wi-Fi") : qsTr("Wired")); font.bold: true }
   Note { text: Number(device.modelData.state) === 100 ? qsTr("Connected") : Number(device.modelData.state) >= 40 && Number(device.modelData.state) < 100 ? qsTr("Connecting…") : qsTr("Disconnected") }
   HarborButton { text: qsTr("Disconnect"); enabled: !NetworkSettings.busy && Number(device.modelData.state) >= 40; onClicked: NetworkSettings.disconnectDevice(device.modelData.path) }
  }
 }
 HarborLabel { text: qsTr("Nearby Wi-Fi networks"); font.bold: true }
 Repeater {
  model: NetworkSettings.networks
  delegate: Card {
   id: ap
   required property var modelData
   HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: ap.modelData.name || qsTr("Hidden network"); font.bold: true }
   Note { text: Number(ap.modelData.strength) + "% · " + (ap.modelData.enterprise ? qsTr("Enterprise") : ap.modelData.secured ? qsTr("Secured") : qsTr("Open")) }
   HarborButton { text: qsTr("Connect"); enabled: !NetworkSettings.busy; onClicked: { if(ap.modelData.enterprise)root.enterprise(ap.modelData.name);else if(ap.modelData.secured)root.selectedAp=ap.modelData.path;else NetworkSettings.connectWifi(ap.modelData.path,"") } }
   ColumnLayout {
    Layout.fillWidth: true; visible: root.selectedAp === ap.modelData.path
    Note { visible: ap.modelData.enterprise; text: qsTr("Use an existing 802.1X profile from Saved connections for enterprise Wi-Fi.") }
    HarborField { id: password; Layout.fillWidth: true; visible: !ap.modelData.enterprise; placeholderText: qsTr("Wi-Fi password"); Accessible.name: placeholderText; echoMode: TextInput.Password }
    RowLayout {
     HarborButton { visible: !ap.modelData.enterprise; text: qsTr("Join network"); enabled: !NetworkSettings.busy; onClicked: {NetworkSettings.connectWifi(ap.modelData.path,password.text);password.text=""} }
     HarborButton { text: qsTr("Cancel"); onClicked: {password.text="";root.selectedAp=""} }
    }
   }
  }
 }
 HarborLabel { text: qsTr("Saved connections"); font.bold: true }
 Repeater {
  model: NetworkSettings.profiles
  delegate: Card {
   id: connection
   required property var modelData
   HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: connection.modelData.name; font.bold: true }
   Note { text: connection.modelData.type }
   Flow {
    Layout.fillWidth: true; spacing: 8
    HarborButton { text: qsTr("Connect"); enabled: !NetworkSettings.busy; onClicked: NetworkSettings.activate(connection.modelData.path) }
    HarborButton { text: qsTr("Edit connection"); enabled: !NetworkSettings.busy; onClicked: root.edit(connection.modelData.path) }
   }
  }
 }
 HarborButton { text: qsTr("New enterprise Wi-Fi"); enabled: NetworkSettings.available && !NetworkSettings.busy; onClicked: root.enterprise("") }
 HarborButton { text: qsTr("New wired connection"); enabled: NetworkSettings.available && !NetworkSettings.busy; onClicked: root.edit("") }
 Card {
  visible: root.editing
  HarborLabel { text: qsTr("Connection settings"); font.bold: true }
  HarborField { Layout.fillWidth: true; text: root.draft.name || ""; placeholderText: qsTr("Connection name"); Accessible.name: placeholderText; onTextEdited: root.stage("name",text) }
  HarborField { Layout.fillWidth: true; visible: root.editingPath === ""; text: root.draft.interface || ""; placeholderText: qsTr("Interface (optional, e.g. eth0)"); Accessible.name: placeholderText; onTextEdited: root.stage("interface",text) }
  HarborField { Layout.fillWidth: true; visible: root.editingPath === "" && root.draft.type === "802-11-wireless"; text: root.draft.ssid || ""; placeholderText: qsTr("Wi-Fi network name (SSID)"); Accessible.name: placeholderText; onTextEdited: root.stage("ssid",text) }
  CheckBox { visible: root.draft.type === "802-3-ethernet" && !root.draft.enterprise; text: qsTr("Configure 802.1X authentication"); onClicked: {root.stage("enterprise",true);root.stage("eap","peap");root.stage("phase2Auth","mschapv2")} }
  ColumnLayout {
   visible: !!root.draft.enterprise; Layout.fillWidth: true
   HarborLabel { text: qsTr("Enterprise authentication · 802.1X"); font.bold: true }
   Note { visible: root.draft.enterpriseEditable === false; text: qsTr("This profile uses advanced authentication managed outside Harbor. Its authentication settings are preserved.") }
   ColumnLayout {
    visible: root.draft.enterpriseEditable !== false; Layout.fillWidth: true
    ComboBox { Layout.fillWidth: true; model: ["PEAP","TTLS","TLS"]; currentIndex: ["peap","ttls","tls"].indexOf(root.draft.eap || "peap"); Accessible.name: qsTr("EAP authentication"); onActivated: {root.stage("eap",["peap","ttls","tls"][currentIndex]);root.stage("phase2Auth","mschapv2")} }
    Repeater {
     model: [{key:"identity",en:QT_TR_NOOP("Identity")},{key:"anonymousIdentity",en:QT_TR_NOOP("Anonymous identity (optional)")},{key:"domainSuffixMatch",en:QT_TR_NOOP("Authentication server domain")},{key:"caCert",en:QT_TR_NOOP("CA certificate file path")}]
     delegate: HarborField { required property var modelData; Layout.fillWidth: true; text: root.draft[modelData.key] || ""; placeholderText: qsTr(modelData.en); Accessible.name: placeholderText; onTextEdited: root.stage(modelData.key,text) }
    }
    ComboBox { visible: root.draft.eap !== "tls"; Layout.fillWidth: true; model: root.draft.eap === "ttls" ? ["mschapv2","pap","chap","mschap"] : ["mschapv2","gtc"]; currentIndex: Math.max(0,model.indexOf(root.draft.phase2Auth || "mschapv2")); Accessible.name: qsTr("Inner authentication"); onActivated: root.stage("phase2Auth",model[currentIndex]) }
    HarborField { visible: root.draft.eap !== "tls"; Layout.fillWidth: true; text: root.draft.eapPassword || ""; placeholderText: qsTr("Password (blank keeps saved password or asks on connect)"); Accessible.name: placeholderText; echoMode: TextInput.Password; onTextEdited: root.stage("eapPassword",text) }
    Repeater {
     model: [{key:"clientCert",en:QT_TR_NOOP("Client certificate file path")},{key:"privateKey",en:QT_TR_NOOP("Private key file path")}]
     delegate: HarborField { required property var modelData; visible: root.draft.eap === "tls"; Layout.fillWidth: true; text: root.draft[modelData.key] || ""; placeholderText: qsTr(modelData.en); Accessible.name: placeholderText; onTextEdited: root.stage(modelData.key,text) }
    }
    HarborField { visible: root.draft.eap === "tls"; Layout.fillWidth: true; text: root.draft.privateKeyPassword || ""; placeholderText: qsTr("Private key password (blank to keep)"); Accessible.name: placeholderText; echoMode: TextInput.Password; onTextEdited: root.stage("privateKeyPassword",text) }
    Note { text: qsTr("Use the CA certificate and server domain provided by your administrator. Certificate verification is required. Keep certificate files in a permanent location. New credentials are saved by NetworkManager; new profiles are restricted to your user.") }
   }
  }
  Repeater {
   model: ["ipv4", "ipv6"]
   delegate: ColumnLayout {
    id: ip
    required property string modelData
    Layout.fillWidth: true; spacing: 8
    HarborLabel { text: ip.modelData.toUpperCase(); font.bold: true }
    ComboBox { Layout.fillWidth: true; model: [qsTr("Automatic"),qsTr("Manual"),qsTr("Disabled")]; currentIndex: ["auto","manual","disabled"].indexOf(root.draft[ip.modelData+"Method"] || "auto"); Accessible.name: ip.modelData + qsTr(" addressing"); onActivated: root.stage(ip.modelData+"Method",["auto","manual","disabled"][currentIndex]) }
    HarborField { Layout.fillWidth: true; visible: root.draft[ip.modelData+"Method"] === "manual"; text: root.draft[ip.modelData+"Addresses"] || ""; placeholderText: ip.modelData === "ipv4" ? "192.168.1.10/24" : "2001:db8::10/64"; Accessible.name: ip.modelData + qsTr(" addresses and prefixes"); onTextEdited: root.stage(ip.modelData+"Addresses",text) }
    HarborField { Layout.fillWidth: true; visible: root.draft[ip.modelData+"Method"] === "manual"; text: root.draft[ip.modelData+"Gateway"] || ""; placeholderText: qsTr("Gateway (optional)"); Accessible.name: ip.modelData + placeholderText; onTextEdited: root.stage(ip.modelData+"Gateway",text) }
    CheckBox { text: qsTr("Use automatic DNS"); checked: !root.draft[ip.modelData+"ManualDns"]; enabled: root.draft[ip.modelData+"Method"] !== "disabled"; onClicked: root.stage(ip.modelData+"ManualDns",!checked) }
    HarborField { Layout.fillWidth: true; enabled: root.draft[ip.modelData+"Method"] !== "disabled"; text: root.draft[ip.modelData+"Dns"] || ""; placeholderText: qsTr("DNS servers, separated by commas"); Accessible.name: ip.modelData + placeholderText; onTextEdited: root.stage(ip.modelData+"Dns",text) }
   }
  }
  ColumnLayout {
   visible: !!root.draft.vpnOpenvpn; Layout.fillWidth: true
   HarborLabel { text: qsTr("VPN sign-in"); font.bold: true }
   HarborField { Layout.fillWidth: true; text: root.draft.vpnUsername || ""; placeholderText: qsTr("VPN username"); Accessible.name: placeholderText; onTextEdited: root.stage("vpnUsername",text) }
   HarborField { id: vpnPassword; Layout.fillWidth: true; placeholderText: qsTr("New password (leave blank to keep)"); Accessible.name: placeholderText; echoMode: TextInput.Password; onTextEdited: root.stage("vpnPassword",text) }
   Note { text: qsTr("A new password is saved by NetworkManager with this connection. Imported VPNs are restricted to your user.") }
  }
  Note { text: qsTr("Saving changes the profile. Connect again to apply it to the active network.") }
  RowLayout {
   HarborButton { text: qsTr("Save profile"); enabled: !NetworkSettings.busy; onClicked: {NetworkSettings.saveProfile(root.editingPath,root.draft);root.stage("vpnPassword","");root.stage("eapPassword","");root.stage("privateKeyPassword","");vpnPassword.text=""} }
   HarborButton { text: qsTr("Close"); onClicked: {root.editing=false;root.draft={}} }
  }
 }
 Card {
  HarborLabel { text: qsTr("Import VPN"); font.bold: true }
  Note { text: qsTr("Import a local OpenVPN or WireGuard configuration. The matching NetworkManager plugin must be installed. Imported profiles are restricted to your user.") }
  ComboBox { id: vpnType; Layout.fillWidth: true; model: ["OpenVPN","WireGuard"]; Accessible.name: qsTr("VPN type") }
  HarborField { id: vpnFile; Layout.fillWidth: true; placeholderText: qsTr("Configuration file path"); Accessible.name: placeholderText }
  HarborButton { text: qsTr("Import"); enabled: NetworkSettings.available && !NetworkSettings.busy && vpnFile.text.length > 0; onClicked: NetworkSettings.importVpn(vpnFile.text,vpnType.currentIndex === 0 ? "openvpn" : "wireguard") }
 }
 Note { visible: NetworkSettings.busy; text: qsTr("Updating network settings…") }
 Note { visible: NetworkSettings.error.length > 0; text: NetworkSettings.error; color: Prefs.dark ? "#ffaaa4" : "#b22b24" }
}
