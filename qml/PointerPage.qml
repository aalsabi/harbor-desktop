pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id:root
 property string deviceType:"mouse"
 property color ink:Prefs.dark?"#eeeef0":"#252527"
 property color muted:Prefs.dark?"#a6a6ad":"#76767c"
 property color card:Prefs.dark?"#303034":"#ffffff"
 property color line:Prefs.dark?"#454549":"#e2e2e7"
 property var devices:PointerSettings.devices.filter(d=>root.deviceType==="touchpad"?d.touchpad:!d.touchpad)
 spacing:18
 LayoutMirroring.enabled:Prefs.language==="ar"
 LayoutMirroring.childrenInherit:true
 Component.onCompleted:PointerSettings.setActive(true)
 Component.onDestruction:PointerSettings.setActive(false)
 component Label:Text {Layout.fillWidth:true;color:root.ink;font.pixelSize:13;wrapMode:Text.WordWrap}
 component Note:Label {color:root.muted;font.pixelSize:12}
 component Group:Rectangle {
  default property alias contents:body.data
  Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:root.card;border.color:root.line
  ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:12}
 }
 Note{text:root.deviceType==="touchpad"?qsTr("Adjust scrolling, tracking and gestures for each touchpad."):qsTr("Adjust scrolling and tracking for each connected mouse.")}
 Group {
  visible:!PointerSettings.available||root.devices.length===0
  Label{text:!PointerSettings.available?qsTr("Pointer settings are unavailable in this session."):root.deviceType==="touchpad"?qsTr("No touchpad connected"):qsTr("No mouse connected")}
  HarborButton{text:qsTr("Refresh");enabled:!PointerSettings.busy;onClicked:PointerSettings.refresh()}
 }
 Group {
  visible:PointerSettings.error.length>0
  Label{text:qsTr("Could not update pointer settings");font.bold:true}
  Note{text:PointerSettings.error}
 }
 Repeater {
  model:root.devices
  delegate:Group {
   id:deviceCard
   required property var modelData
   Label{text:deviceCard.modelData.name||deviceCard.modelData.sysName;font.bold:true;font.pixelSize:15}
   Note{visible:deviceCard.modelData.enabled===false;text:qsTr("This device is currently disabled by the system.")}
   ColumnLayout {
    Layout.fillWidth:true
    visible:deviceCard.modelData.can_pointerAcceleration===true
    Label{text:qsTr("Tracking speed")}
    Slider {
     id:speed
     objectName:"pointer-speed-"+deviceCard.modelData.sysName
     Layout.fillWidth:true;from:-1;to:1;stepSize:.05
     property real requestedValue:0
     value:deviceCard.modelData.pointerAcceleration||0
     enabled:!PointerSettings.busy
     Accessible.name:qsTr("Tracking speed")
     onMoved:{requestedValue=value;if(!pressed)keyboardCommit.restart()}
     onPressedChanged:{
      if(pressed){requestedValue=value;keyboardCommit.stop();PointerSettings.setActive(false)}
      else {if(enabled)PointerSettings.setSetting(deviceCard.modelData.sysName,"pointerAcceleration",requestedValue);PointerSettings.setActive(true)}
     }
     Timer{id:keyboardCommit;interval:180;onTriggered:PointerSettings.setSetting(deviceCard.modelData.sysName,"pointerAcceleration",speed.requestedValue)}
    }
    RowLayout{Layout.fillWidth:true;Note{text:qsTr("Slow")}Note{text:qsTr("Fast");horizontalAlignment:Text.AlignRight}}
   }
   Repeater {
    model:[{key:"naturalScroll",en:QT_TR_NOOP("Natural scrolling")},{key:"leftHanded",en:QT_TR_NOOP("Left-handed buttons")},{key:"tapToClick",en:QT_TR_NOOP("Tap to click")},{key:"tapAndDrag",en:QT_TR_NOOP("Tap and drag")},{key:"disableWhileTyping",en:QT_TR_NOOP("Ignore touchpad while typing")}]
    delegate:RowLayout {
     id:settingRow
     required property var modelData
     Layout.fillWidth:true
     visible:deviceCard.modelData["can_"+settingRow.modelData.key]===true
     Label{text:qsTr(settingRow.modelData.en)}
     Switch {
      objectName:"pointer-"+settingRow.modelData.key+"-"+deviceCard.modelData.sysName
      checked:deviceCard.modelData[settingRow.modelData.key]===true
      enabled:!PointerSettings.busy&&(settingRow.modelData.key!=="tapAndDrag"||deviceCard.modelData.tapToClick===true)
      Accessible.name:qsTr(settingRow.modelData.en)
      onClicked:{PointerSettings.setSetting(deviceCard.modelData.sysName,settingRow.modelData.key,checked);checked=Qt.binding(function(){return deviceCard.modelData[settingRow.modelData.key]===true})}
     }
    }
   }
   Note{visible:!deviceCard.modelData.can_pointerAcceleration&&!deviceCard.modelData.can_naturalScroll&&!deviceCard.modelData.can_leftHanded&&!deviceCard.modelData.can_tapToClick&&!deviceCard.modelData.can_disableWhileTyping;text:qsTr("This device does not expose adjustable pointer settings.")}
  }
 }
 Note{visible:root.devices.length>0;text:qsTr("Changes apply immediately and are saved by the system.")}
}
