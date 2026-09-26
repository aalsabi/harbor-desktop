import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout{
 id:root;objectName:"accessibilityPage";spacing:14;function t(en,ar){return Prefs.language==="ar"?ar:en}
 Component.onCompleted:AccessibilitySettings.refresh()
 HarborLabel{text:root.t("Vision and screen reading","الرؤية وقراءة الشاشة");font.bold:true;font.pixelSize:17}
 Repeater{model:[{id:"zoom",en:"Screen magnification",ar:"تكبير الشاشة"},{id:"invert",en:"Invert colors",ar:"عكس الألوان"},{id:"trackmouse",en:"Enable pointer locator shortcut",ar:"تفعيل اختصار تحديد موقع المؤشر"},{id:"mouseclick",en:"Enable mouse-click effect shortcut",ar:"تفعيل اختصار مؤثر نقرات الماوس"}];delegate:RowLayout{required property var modelData;Layout.fillWidth:true;HarborLabel{Layout.fillWidth:true;text:root.t(modelData.en,modelData.ar)}Switch{checked:(AccessibilitySettings.state[modelData.id==="invert"?"active":"loaded"]||[]).includes(modelData.id);enabled:!AccessibilitySettings.busy&&(AccessibilitySettings.state.available||[]).includes(modelData.id);Accessible.name:root.t(modelData.en,modelData.ar);onClicked:{AccessibilitySettings.setEffect(modelData.id,checked);checked=Qt.binding(function(){return (AccessibilitySettings.state[modelData.id==="invert"?"active":"loaded"]||[]).includes(modelData.id)})}}}}
 RowLayout{HarborButton{text:root.t("Zoom in","تكبير");enabled:!AccessibilitySettings.busy&&(AccessibilitySettings.state.loaded||[]).includes("zoom");onClicked:AccessibilitySettings.zoom("in")}HarborButton{text:root.t("Zoom out","تصغير");enabled:!AccessibilitySettings.busy&&(AccessibilitySettings.state.loaded||[]).includes("zoom");onClicked:AccessibilitySettings.zoom("out")}HarborButton{text:root.t("Actual size","الحجم الفعلي");enabled:!AccessibilitySettings.busy&&(AccessibilitySettings.state.loaded||[]).includes("zoom");onClicked:AccessibilitySettings.zoom("reset")}}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.t("Visual aids depend on compositor support. Pointer location and click effects use their configured KWin shortcuts.","تعتمد المساعدات البصرية على دعم مدير النوافذ. تستخدم مؤثرات المؤشر والنقرات اختصارات KWin المضبوطة.")}
 RowLayout{Layout.fillWidth:true;HarborLabel{Layout.fillWidth:true;text:root.t("Orca screen reader","قارئ الشاشة Orca")}Switch{checked:!!AccessibilitySettings.state.screenReaderEnabled;enabled:!!AccessibilitySettings.state.orcaAvailable;Accessible.name:root.t("Screen reader","قارئ الشاشة");onClicked:AccessibilitySettings.setScreenReader(checked)}}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:!AccessibilitySettings.state.orcaAvailable?root.t("Install Orca to enable screen reading.","ثبّت Orca لتفعيل قراءة الشاشة."):(AccessibilitySettings.state.readerStatus||"")}
 HarborButton{text:root.t("Refresh","تحديث");enabled:!AccessibilitySettings.busy;onClicked:AccessibilitySettings.refresh()}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:AccessibilitySettings.error}
}
