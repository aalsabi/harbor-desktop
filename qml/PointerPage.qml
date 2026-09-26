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
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 Component.onCompleted:PointerSettings.setActive(true)
 Component.onDestruction:PointerSettings.setActive(false)
 component Label:Text {Layout.fillWidth:true;color:root.ink;font.pixelSize:13;wrapMode:Text.WordWrap}
 component Note:Label {color:root.muted;font.pixelSize:12}
 component Group:Rectangle {
  default property alias contents:body.data
  Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:root.card;border.color:root.line
  ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:12}
 }
 Note{text:root.deviceType==="touchpad"?root.t("Adjust scrolling, tracking and gestures for each touchpad.","اضبط التمرير وسرعة المؤشر وإيماءات كل لوحة لمس."):root.t("Adjust scrolling and tracking for each connected mouse.","اضبط التمرير وسرعة المؤشر لكل فأرة متصلة.")}
 Group {
  visible:!PointerSettings.available||root.devices.length===0
  Label{text:!PointerSettings.available?root.t("Pointer settings are unavailable in this session.","إعدادات المؤشر غير متاحة في هذه الجلسة."):root.deviceType==="touchpad"?root.t("No touchpad connected","لا توجد لوحة لمس متصلة"):root.t("No mouse connected","لا توجد فأرة متصلة")}
  HarborButton{text:root.t("Refresh","تحديث");enabled:!PointerSettings.busy;onClicked:PointerSettings.refresh()}
 }
 Group {
  visible:PointerSettings.error.length>0
  Label{text:root.t("Could not update pointer settings","تعذّر تحديث إعدادات المؤشر");font.bold:true}
  Note{text:PointerSettings.error}
 }
 Repeater {
  model:root.devices
  delegate:Group {
   id:deviceCard
   required property var modelData
   Label{text:deviceCard.modelData.name||deviceCard.modelData.sysName;font.bold:true;font.pixelSize:15}
   Note{visible:deviceCard.modelData.enabled===false;text:root.t("This device is currently disabled by the system.","هذا الجهاز معطّل حاليًا بواسطة النظام.")}
   ColumnLayout {
    Layout.fillWidth:true
    visible:deviceCard.modelData.can_pointerAcceleration===true
    Label{text:root.t("Tracking speed","سرعة المؤشر")}
    Slider {
     id:speed
     objectName:"pointer-speed-"+deviceCard.modelData.sysName
     Layout.fillWidth:true;from:-1;to:1;stepSize:.05
     property real requestedValue:0
     value:deviceCard.modelData.pointerAcceleration||0
     enabled:!PointerSettings.busy
     Accessible.name:root.t("Tracking speed","سرعة المؤشر")
     onMoved:{requestedValue=value;if(!pressed)keyboardCommit.restart()}
     onPressedChanged:{
      if(pressed){requestedValue=value;keyboardCommit.stop();PointerSettings.setActive(false)}
      else {if(enabled)PointerSettings.setSetting(deviceCard.modelData.sysName,"pointerAcceleration",requestedValue);PointerSettings.setActive(true)}
     }
     Timer{id:keyboardCommit;interval:180;onTriggered:PointerSettings.setSetting(deviceCard.modelData.sysName,"pointerAcceleration",speed.requestedValue)}
    }
    RowLayout{Layout.fillWidth:true;Note{text:root.t("Slow","بطيء")}Note{text:root.t("Fast","سريع");horizontalAlignment:Text.AlignRight}}
   }
   Repeater {
    model:[{key:"naturalScroll",en:"Natural scrolling",ar:"التمرير الطبيعي"},{key:"leftHanded",en:"Left-handed buttons",ar:"أزرار لليد اليسرى"},{key:"tapToClick",en:"Tap to click",ar:"النقر باللمس"},{key:"tapAndDrag",en:"Tap and drag",ar:"النقر والسحب"},{key:"disableWhileTyping",en:"Ignore touchpad while typing",ar:"تجاهل لوحة اللمس أثناء الكتابة"}]
    delegate:RowLayout {
     id:settingRow
     required property var modelData
     Layout.fillWidth:true
     visible:deviceCard.modelData["can_"+settingRow.modelData.key]===true
     Label{text:root.t(settingRow.modelData.en,settingRow.modelData.ar)}
     Switch {
      objectName:"pointer-"+settingRow.modelData.key+"-"+deviceCard.modelData.sysName
      checked:deviceCard.modelData[settingRow.modelData.key]===true
      enabled:!PointerSettings.busy&&(settingRow.modelData.key!=="tapAndDrag"||deviceCard.modelData.tapToClick===true)
      Accessible.name:root.t(settingRow.modelData.en,settingRow.modelData.ar)
      onClicked:{PointerSettings.setSetting(deviceCard.modelData.sysName,settingRow.modelData.key,checked);checked=Qt.binding(function(){return deviceCard.modelData[settingRow.modelData.key]===true})}
     }
    }
   }
   Note{visible:!deviceCard.modelData.can_pointerAcceleration&&!deviceCard.modelData.can_naturalScroll&&!deviceCard.modelData.can_leftHanded&&!deviceCard.modelData.can_tapToClick&&!deviceCard.modelData.can_disableWhileTyping;text:root.t("This device does not expose adjustable pointer settings.","هذا الجهاز لا يتيح إعدادات مؤشر قابلة للتعديل.")}
  }
 }
 Note{visible:root.devices.length>0;text:root.t("Changes apply immediately and are saved by the system.","تُطبّق التغييرات فورًا ويحفظها النظام.")}
}
