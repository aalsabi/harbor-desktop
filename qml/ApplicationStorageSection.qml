pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id: root
 objectName: "applicationStorageSection"
 spacing: 12
 readonly property bool arabic: Prefs.language === "ar"
 readonly property color muted: Prefs.dark ? "#aaaab2" : "#696971"
 property int shown: 20
 property var pendingAssociation: ({})
 readonly property var matching: ApplicationStorage.applications.filter(function(app){return app.name.toLowerCase().indexOf(filter.text.toLowerCase())>=0})
 function size(value) { var amount=Number(value || 0),units=["B","KiB","MiB","GiB","TiB"],i=0;while(amount>=1024&&i<units.length-1){amount/=1024;++i}return amount.toFixed(i?1:0)+" "+units[i] }
 LayoutMirroring.enabled: arabic
 LayoutMirroring.childrenInherit: true
 component Note: HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: root.muted; textFormat: Text.PlainText }
 component Card: Rectangle {
  default property alias contents: layout.data
  Layout.fillWidth: true; implicitHeight: layout.implicitHeight+28
  radius: 12; color: Prefs.dark ? "#29292f" : "#ffffff"; border.color: Prefs.dark ? "#45454d" : "#dedee4"
  ColumnLayout { id: layout; anchors { left: parent.left; right: parent.right; top: parent.top; margins: 14 } spacing: 8 }
 }
 HarborLabel { text: qsTr("Measured application storage"); font.pixelSize: 17; font.bold: true; Layout.fillWidth: true; wrapMode: Text.WordWrap }
 Note { text: qsTr("Measure installed application files, data, and cache using allocated disk blocks. Nothing is deleted. The scan starts only when you choose Measure.") }
 Flow {
  Layout.fillWidth: true; spacing: 8
  HarborButton { text: qsTr("Measure application storage"); enabled: !ApplicationStorage.busy; onClicked: {root.shown=20;ApplicationStorage.measure()} }
  HarborButton { visible: ApplicationStorage.busy; text: qsTr("Cancel scan"); onClicked: ApplicationStorage.cancel() }
 }
 Note { visible: ApplicationStorage.message.length>0; text: ApplicationStorage.busy ? qsTr("Reading and measuring application files… ")+ApplicationStorage.scannedFiles : ApplicationStorage.summary.canceled ? qsTr("Canceled. These are partial results.") : (ApplicationStorage.summary.capped || ApplicationStorage.summary.inventoryLimited) ? qsTr("Scan limit reached. These are partial results.") : qsTr("Measurement complete: ")+(ApplicationStorage.summary.scannedFiles || 0)+qsTr(" unique files.") }
 Note { visible: ApplicationStorage.busy && Object.keys(ApplicationStorage.summary).length>0; text: qsTr("Previous results remain visible until this scan finishes.") }
 Note { visible: ApplicationStorage.error.length>0; text: ApplicationStorage.error; color: Prefs.dark ? "#ffaaa4" : "#b22b24" }
 Card {
  visible: Object.keys(ApplicationStorage.summary).length>0
  HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; font.bold: true; text: qsTr("Measured total: ")+root.size(ApplicationStorage.summary.measuredBytes) }
  Note { text: qsTr("Shared files, counted once: ")+root.size(ApplicationStorage.summary.sharedBytes) }
  Note { text: qsTr("Unattributed data and settings: ")+root.size(ApplicationStorage.summary.unattributedDataBytes) }
  Note { text: qsTr("Unattributed cache: ")+root.size(ApplicationStorage.summary.unattributedCacheBytes) }
  Note { text: qsTr("Excluded links, unavailable paths, or unreadable entries: ")+(ApplicationStorage.summary.skipped || 0) }
 }
 Note { text: qsTr("Installed files come from the package owning each desktop application or its Flatpak deployment. Packages used by several apps and hard-linked files are shown separately as shared. Dependencies without a desktop app and Flatpak runtimes are excluded. Unattributed totals cover standard XDG folders, not your whole home. Directory metadata and symlinks are excluded; reflinks and compression can make physical disk usage differ. These figures are not a reclaimable-space estimate.") }
 HarborField { id: filter; Layout.fillWidth: true; visible: ApplicationStorage.applications.length>0; placeholderText: qsTr("Find an application"); Accessible.name: placeholderText; onTextEdited: root.shown=20 }
 Repeater {
  model: root.matching.slice(0,root.shown)
  delegate: Card {
   id: app
   required property var modelData
   HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; textFormat: Text.PlainText; font.bold: true; text: app.modelData.name }
   Note { text: qsTr("Attributed total: ")+root.size(app.modelData.totalBytes) }
   Note { text: qsTr("Installed files: ")+(app.modelData.installedKnown ? root.size(app.modelData.installedBytes) : qsTr("Unknown — package ownership was not found")) }
   Note { text: qsTr("Data and settings: ")+(app.modelData.dataKnown ? root.size(app.modelData.dataBytes) : qsTr("Not attributed; associate a folder below")) }
   Note { text: qsTr("Cache: ")+(app.modelData.cacheKnown ? root.size(app.modelData.cacheBytes) : qsTr("Not attributed; associate a folder below")) }
   Note { visible: app.modelData.sharedReferencedBytes>0; text: qsTr("References shared files: ")+root.size(app.modelData.sharedReferencedBytes)+qsTr(" (excluded from this app’s total)") }
  }
 }
 HarborButton { visible: root.matching.length>root.shown; text: qsTr("Show more applications"); onClicked: root.shown+=20 }
 Card {
  visible: ApplicationStorage.applications.length>0
  HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; font.bold: true; text: qsTr("Associate a native app folder") }
  Note { text: qsTr("Native applications do not reliably identify their data folders. Choose a folder inside your home directory only when you know it belongs to the selected app. Flatpak data, settings, and cache are mapped automatically.") }
  ComboBox { id: choice; Layout.fillWidth: true; model: ApplicationStorage.applications; textRole: "name"; valueRole: "id"; Accessible.name: qsTr("Application") }
  ComboBox { id: kind; Layout.fillWidth: true; model: [qsTr("Data and settings"),qsTr("Cache")]; Accessible.name: qsTr("Folder type") }
  HarborField { id: folder; Layout.fillWidth: true; placeholderText: qsTr("Absolute folder path inside your home"); Accessible.name: placeholderText }
  HarborButton { text: qsTr("Review association"); enabled: !ApplicationStorage.busy&&choice.currentIndex>=0&&folder.text.length>0; onClicked: root.pendingAssociation={id:choice.currentValue,name:choice.currentText,path:folder.text,kind:kind.currentIndex===0?"data":"cache"} }
  ColumnLayout {
   Layout.fillWidth: true; visible: !!root.pendingAssociation.id
   Note { text: qsTr("Associate this folder and its regular files with ")+(root.pendingAssociation.name || "")+"?\n"+(root.pendingAssociation.path || "") }
   Flow {
    Layout.fillWidth: true; spacing: 8
    HarborButton { text: qsTr("Confirm and measure"); enabled: !ApplicationStorage.busy; onClicked: {ApplicationStorage.associateFolder(root.pendingAssociation.id,root.pendingAssociation.path,root.pendingAssociation.kind);root.pendingAssociation={}} }
    HarborButton { text: qsTr("Cancel"); onClicked: root.pendingAssociation={} }
   }
  }
 }
 Repeater {
  model: ApplicationStorage.associations
  delegate: ColumnLayout {
   id: association
   required property var modelData
   Layout.fillWidth: true
   Note { text: (ApplicationStorage.applications.find(function(app){return app.id===association.modelData.id}) || ({name:association.modelData.id})).name+" · "+(association.modelData.kind==="cache"?qsTr("Cache"):qsTr("Data and settings"))+"\n"+association.modelData.path; wrapMode: Text.WrapAnywhere }
   HarborButton { text: qsTr("Remove association"); enabled: !ApplicationStorage.busy; onClicked: ApplicationStorage.removeAssociation(association.modelData.id,association.modelData.path) }
  }
 }
}
