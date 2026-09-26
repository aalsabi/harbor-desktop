pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id:root;spacing:16
 property color ink:Prefs.dark?"#eeeef0":"#252527"
 property color muted:Prefs.dark?"#a6a6ad":"#76767c"
 property var draft:({})
 property bool draftEnabled:false
 property bool hasTouchpad:PointerSettings.devices.some(d=>d.touchpad&&d.gestureSupport)
 property var actions:[{key:"none",en:"No custom action",ar:"لا إجراء مخصص"},{key:"launcher",en:"Application launcher",ar:"مشغّل التطبيقات"},{key:"control",en:"Control Center",ar:"مركز التحكم"},{key:"notifications",en:"Notifications",ar:"الإشعارات"},{key:"settings",en:"Settings",ar:"الإعدادات"},{key:"windows",en:"Window list",ar:"قائمة النوافذ"}]
 property var rows:[{key:"3-Up",en:"Three fingers up",ar:"ثلاثة أصابع لأعلى"},{key:"3-Down",en:"Three fingers down",ar:"ثلاثة أصابع لأسفل"},{key:"3-Left",en:"Three fingers left",ar:"ثلاثة أصابع لليسار"},{key:"3-Right",en:"Three fingers right",ar:"ثلاثة أصابع لليمين"},{key:"4-Up",en:"Four fingers up",ar:"أربعة أصابع لأعلى"},{key:"4-Down",en:"Four fingers down",ar:"أربعة أصابع لأسفل"},{key:"4-Left",en:"Four fingers left",ar:"أربعة أصابع لليسار"},{key:"4-Right",en:"Four fingers right",ar:"أربعة أصابع لليمين"}]
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 function reload(){draft=Object.assign({},GestureSettings.mappings);draftEnabled=GestureSettings.enabled}
 Component.onCompleted:{reload();GestureSettings.refresh();PointerSettings.setActive(true)}
 Component.onDestruction:PointerSettings.setActive(false)
 Connections{target:GestureSettings;function onChanged(){if(!GestureSettings.busy)root.reload()}}
 Text{Layout.fillWidth:true;wrapMode:Text.WordWrap;color:root.muted;text:root.t("Custom touchpad swipes are off by default. KWin's built-in gestures may also run for the same swipe. Use only the actions you want and disable custom gestures if they conflict.","إيماءات لوحة اللمس المخصصة معطّلة افتراضيًا. قد تعمل إيماءات KWin المدمجة مع التمرير نفسه. اختر الإجراءات المطلوبة فقط وعطّل الإيماءات المخصصة عند التعارض.")}
 Text{Layout.fillWidth:true;visible:!root.hasTouchpad;wrapMode:Text.WordWrap;color:root.muted;text:root.t("No gesture-capable touchpad is currently detected. Saved mappings will work when a compatible touchpad is connected.","لم يتم اكتشاف لوحة لمس تدعم الإيماءات حاليًا. تعمل التعيينات المحفوظة عند توصيل لوحة لمس متوافقة.")}
 Rectangle{Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7"
  ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:10
   Switch{text:root.t("Enable custom gestures","تفعيل الإيماءات المخصصة");checked:root.draftEnabled;enabled:GestureSettings.available&&!GestureSettings.busy;onClicked:root.draftEnabled=checked}
   Repeater{model:root.rows;delegate:RowLayout{
    id:row;required property var modelData;Layout.fillWidth:true
    Text{Layout.fillWidth:true;color:root.ink;text:root.t(row.modelData.en,row.modelData.ar);wrapMode:Text.WordWrap}
    ComboBox{Layout.preferredWidth:190;model:root.actions.map(a=>root.t(a.en,a.ar));currentIndex:Math.max(0,root.actions.findIndex(a=>a.key===(root.draft[row.modelData.key]||"none")));enabled:!GestureSettings.busy;onActivated:{let copy=Object.assign({},root.draft);copy[row.modelData.key]=root.actions[currentIndex].key;root.draft=copy}}
   }}
  }
 }
 Text{Layout.fillWidth:true;wrapMode:Text.WordWrap;color:root.muted;text:GestureSettings.error||(GestureSettings.active?root.t("Custom gesture handlers are active.","معالجات الإيماءات المخصصة نشطة."):root.t("Custom gesture handlers are not active.","معالجات الإيماءات المخصصة غير نشطة."))}
 RowLayout{HarborButton{text:root.t("Apply gestures","تطبيق الإيماءات");enabled:GestureSettings.available&&!GestureSettings.busy;onClicked:GestureSettings.apply(root.draft,root.draftEnabled)}HarborButton{text:root.t("Reload","إعادة التحميل");enabled:!GestureSettings.busy;onClicked:GestureSettings.refresh()}}
}
