import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout{
 id:root;spacing:16
 property color ink:Prefs.dark?"#eeeeef":"#252527"
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 component Card:Rectangle{default property alias contents:body.data;Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7";ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:12}}
 component Label:Text{Layout.fillWidth:true;wrapMode:Text.Wrap;color:root.ink;font.pixelSize:13;textFormat:Text.PlainText}
 FocusSection{Layout.fillWidth:true}
 Card{RowLayout{Layout.fillWidth:true;Label{text:root.t("Do Not Disturb","عدم الإزعاج");font.bold:true}Switch{checked:NotificationPrefs.doNotDisturb;Accessible.name:root.t("Do Not Disturb","عدم الإزعاج");onClicked:NotificationPrefs.doNotDisturb=checked}}Label{text:root.t("Hide notification attention in the menu bar. Notifications remain available in Notification Center.","إخفاء مؤشر التنبيهات من الشريط العلوي. تبقى الإشعارات متاحة في مركز الإشعارات.")}}
 Label{text:root.t("Applications appear after sending a notification. Settings apply to Harbor's notification service.","تظهر التطبيقات بعد إرسال إشعار. تنطبق الخيارات على خدمة إشعارات Harbor.")}
 Repeater{model:NotificationPrefs.applications;delegate:Card{required property var modelData
  Label{text:modelData.name;font.bold:true}
  RowLayout{Layout.fillWidth:true;Label{text:root.t("Allow notifications","السماح بالإشعارات")}Switch{checked:modelData.enabled;Accessible.name:root.t("Allow notifications","السماح بالإشعارات")+" "+modelData.name;onClicked:NotificationPrefs.setEnabled(modelData.id,checked)}}
  RowLayout{Layout.fillWidth:true;Label{text:root.t("Show message previews","إظهار معاينة الرسائل")}Switch{checked:modelData.preview;enabled:modelData.enabled;Accessible.name:root.t("Show previews","إظهار المعاينات")+" "+modelData.name;onClicked:NotificationPrefs.setPreview(modelData.id,checked)}}
 }}
 Label{visible:NotificationPrefs.applications.length===0;text:root.t("No applications have sent notifications yet.","لم ترسل التطبيقات إشعارات بعد.")}
}
