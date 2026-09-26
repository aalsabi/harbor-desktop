pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id:root;spacing:16
 property color ink:Prefs.dark?"#eeeef0":"#252527"
 property color muted:Prefs.dark?"#a6a6ad":"#76767c"
 Component.onCompleted:PowerSettings.setActive(true)
 Component.onDestruction:PowerSettings.setActive(false)
 component Label:Text{Layout.fillWidth:true;wrapMode:Text.WordWrap;color:root.ink;font.pixelSize:13}
 component Group:Rectangle{default property alias contents:body.data;Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7";ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:12}}
 Group{
  Label{text:qsTr("When idle");font.bold:true;font.pixelSize:15}
  Label{text:qsTr("Timeouts apply to the current Harbor session. Zero means never; activity wakes the displays. Applications can inhibit idle actions.");color:root.muted}
  RowLayout{Layout.fillWidth:true;Label{text:qsTr("Turn displays off after (minutes)")}SpinBox{id:displayTimeout;from:0;to:240;value:PowerSettings.displayMinutes;editable:true;enabled:PowerSettings.idleAvailable&&!PowerSettings.busy}}
  RowLayout{Layout.fillWidth:true;Label{text:qsTr("Suspend after (minutes)")}SpinBox{id:suspendTimeout;from:0;to:240;value:PowerSettings.suspendMinutes;editable:true;enabled:PowerSettings.idleAvailable&&!PowerSettings.busy}}
  Label{text:PowerSettings.idleStatus;color:root.muted}
  HarborButton{text:qsTr("Apply idle preferences");enabled:PowerSettings.idleAvailable&&!PowerSettings.busy;onClicked:PowerSettings.saveIdle(displayTimeout.value,suspendTimeout.value)}
 }
 Label{visible:PowerSettings.error.length>0;text:PowerSettings.error}
 Repeater{model:PowerSettings.batteries;delegate:Group{
  id:battery;required property var modelData
  Label{text:battery.modelData.name+" · "+battery.modelData.capacity+"% · "+battery.modelData.status;font.bold:true}
  Label{visible:!battery.modelData.canLimit;text:qsTr("This battery does not expose charge limits to the operating system.");color:root.muted}
  ColumnLayout{Layout.fillWidth:true;visible:battery.modelData.canLimit
   RowLayout{Layout.fillWidth:true;visible:battery.modelData.canStart;Label{text:qsTr("Start charging below (%)")}SpinBox{id:start;from:0;to:99;value:Math.max(0,battery.modelData.start);editable:true;enabled:!PowerSettings.busy}}
   RowLayout{Layout.fillWidth:true;Label{text:qsTr("Stop charging at (%)")}SpinBox{id:end;from:1;to:100;value:battery.modelData.end;editable:true;enabled:!PowerSettings.busy}}
   Label{text:qsTr("Administrator authentication is required. Hardware may round values; limits may reset after restarting, depending on the battery driver.");color:root.muted}
   HarborButton{text:qsTr("Apply charge limits");enabled:!PowerSettings.busy&&(!battery.modelData.canStart||start.value<end.value);onClicked:PowerSettings.setChargeLimits(battery.modelData.name,battery.modelData.canStart?start.value:-1,end.value)}
  }
 }}
 Label{visible:PowerSettings.batteries.length===0;text:qsTr("No system battery detected.");color:root.muted}
}
