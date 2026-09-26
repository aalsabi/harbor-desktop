pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id: root
 objectName: "storagePage"
 spacing: 16
 readonly property bool arabic: Prefs.language === "ar"
 readonly property color muted: Prefs.dark ? "#aaaab2" : "#696971"
 function t(en, ar) { return arabic ? ar : en }
 function size(bytes) { var units=["B", "KiB", "MiB", "GiB", "TiB"]; var n=Number(bytes), i=0; while(n>=1024 && i<units.length-1) { n/=1024; ++i } return n.toFixed(i ? 1 : 0) + " " + units[i] }
 LayoutMirroring.enabled: arabic
 LayoutMirroring.childrenInherit: true
 Component.onCompleted: StorageSettings.refresh()
 HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: root.muted; text: root.t("Storage usage for mounted volumes. Available space excludes space reserved by the filesystem.", "استخدام التخزين لوحدات التخزين المتصلة. المساحة المتاحة لا تشمل المساحة المحجوزة لنظام الملفات.") }
 Repeater {
  model: StorageSettings.volumes
  delegate: Rectangle {
   id: volume
   required property var modelData
   Layout.fillWidth: true
   implicitHeight: contents.implicitHeight + 32
   radius: 12; color: Prefs.dark ? "#29292f" : "#ffffff"; border.color: Prefs.dark ? "#45454d" : "#dedee4"
   ColumnLayout {
    id: contents
    anchors { left: parent.left; right: parent.right; top: parent.top; margins: 16 }
    spacing: 10
    HarborLabel { Layout.fillWidth: true; font.bold: true; text: volume.modelData.name; elide: Text.ElideMiddle }
    HarborLabel { Layout.fillWidth: true; text: volume.modelData.mount + " · " + volume.modelData.filesystem + (volume.modelData.readOnly ? root.t(" · Read only", " · للقراءة فقط") : ""); color: root.muted; wrapMode: Text.WrapAnywhere }
    ProgressBar { Layout.fillWidth: true; value: volume.modelData.fraction; Accessible.name: root.t("Storage used", "التخزين المستخدم") }
    HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: root.t("Used: ", "المستخدم: ") + root.size(volume.modelData.used) + root.t(" of ", " من ") + root.size(volume.modelData.total) + root.t(" · Available: ", " · المتاح: ") + root.size(volume.modelData.available) }
    HarborButton { text: root.t("Open folder", "فتح المجلد"); onClicked: StorageSettings.openVolume(volume.modelData.mount) }
   }
  }
 }
 HarborLabel { visible: StorageSettings.volumes.length === 0 && !StorageSettings.busy; text: root.t("No mounted storage volumes found.", "لم يتم العثور على وحدات تخزين متصلة.") }
 HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; visible: StorageSettings.error.length > 0; text: StorageSettings.error; color: Prefs.dark ? "#ffaaa4" : "#b22b24" }
 HarborButton { text: StorageSettings.busy ? root.t("Refreshing…", "جارٍ التحديث…") : root.t("Refresh", "تحديث"); enabled: !StorageSettings.busy; onClicked: StorageSettings.refresh() }

 ApplicationStorageSection { Layout.fillWidth: true }

 HarborLabel {text:root.t("Analyze files", "تحليل الملفات");font.bold:true;font.pixelSize:17}
 HarborLabel {Layout.fillWidth:true;wrapMode:Text.WordWrap;text:root.t("Choose a folder to measure allocated disk space. Links and other filesystems are excluded; no files are deleted.","اختر مجلدًا لحساب المساحة المشغولة. تُستثنى الروابط وأنظمة الملفات الأخرى؛ لن تُحذف ملفات.")}
 HarborField{id:scanPath;Layout.fillWidth:true;text:StorageSettings.homePath;Accessible.name:root.t("Folder to analyze","المجلد المراد تحليله")}
 RowLayout{HarborButton{text:root.t("Analyze","تحليل");enabled:!StorageSettings.analyzing;onClicked:StorageSettings.analyze(scanPath.text)}HarborButton{text:root.t("Cancel","إلغاء");enabled:StorageSettings.analyzing;onClicked:StorageSettings.cancelAnalysis()}}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.WordWrap;text:StorageSettings.analysisMessage}
 Repeater{model:StorageSettings.categories;delegate:HarborLabel{required property var modelData;Layout.fillWidth:true;text:root.t(modelData.name,({Pictures:"صور",Videos:"فيديو",Audio:"صوت",Documents:"مستندات","Archives and packages":"أرشيفات وحزم",Other:"أخرى"})[modelData.name]||modelData.name)+" · "+root.size(modelData.bytes)}}
 HarborLabel{text:root.t("Largest files","أكبر الملفات");font.bold:true;visible:StorageSettings.largestFiles.length>0}
 Repeater{model:StorageSettings.largestFiles;delegate:HarborLabel{required property var modelData;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;text:root.size(modelData.bytes)+" · "+modelData.path}}

}
