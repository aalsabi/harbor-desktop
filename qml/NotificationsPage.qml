import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout{
 id:root;spacing:16
 property color ink:Prefs.dark?"#eeeeef":"#252527"
 component Card:Rectangle{default property alias contents:body.data;Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7";ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:12}}
 component Label:Text{Layout.fillWidth:true;wrapMode:Text.Wrap;color:root.ink;font.pixelSize:13;textFormat:Text.PlainText}
 FocusSection{Layout.fillWidth:true}
 Card{RowLayout{Layout.fillWidth:true;Label{text:qsTr("Do Not Disturb");font.bold:true}Switch{checked:NotificationPrefs.doNotDisturb;Accessible.name:qsTr("Do Not Disturb");onClicked:NotificationPrefs.doNotDisturb=checked}}Label{text:qsTr("Hide notification attention in the menu bar. Notifications remain available in Notification Center.")}}
 Label{text:qsTr("Applications appear after sending a notification. Settings apply to Harbor's notification service.")}
 Repeater{model:NotificationPrefs.applications;delegate:Card{required property var modelData
  Label{text:modelData.name;font.bold:true}
  RowLayout{Layout.fillWidth:true;Label{text:qsTr("Allow notifications")}Switch{checked:modelData.enabled;Accessible.name:qsTr("Allow notifications")+" "+modelData.name;onClicked:NotificationPrefs.setEnabled(modelData.id,checked)}}
  RowLayout{Layout.fillWidth:true;Label{text:qsTr("Show message previews")}Switch{checked:modelData.preview;enabled:modelData.enabled;Accessible.name:qsTr("Show previews")+" "+modelData.name;onClicked:NotificationPrefs.setPreview(modelData.id,checked)}}
 }}
 Label{visible:NotificationPrefs.applications.length===0;text:qsTr("No applications have sent notifications yet.")}
}
