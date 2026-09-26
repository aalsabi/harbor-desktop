pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id:root;spacing:16
 property color ink:Prefs.dark?"#eeeef0":"#252527"
 property color muted:Prefs.dark?"#a6a6ad":"#76767c"
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 Component.onCompleted:PowerSettings.setActive(true)
 Component.onDestruction:PowerSettings.setActive(false)
 component Label:Text{Layout.fillWidth:true;wrapMode:Text.WordWrap;color:root.ink;font.pixelSize:13}
 component Group:Rectangle{default property alias contents:body.data;Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7";ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:12}}
 Group{
  Label{text:root.t("When idle","عند عدم الاستخدام");font.bold:true;font.pixelSize:15}
  Label{text:root.t("Timeouts apply to the current Harbor session. Zero means never; activity wakes the displays. Applications can inhibit idle actions.","تنطبق المدد على جلسة Harbor الحالية. الصفر يعني أبدًا، ويؤدي النشاط إلى إيقاظ الشاشات. يمكن للتطبيقات منع إجراءات الخمول.");color:root.muted}
  RowLayout{Layout.fillWidth:true;Label{text:root.t("Turn displays off after (minutes)","إطفاء الشاشات بعد (دقائق)")}SpinBox{id:displayTimeout;from:0;to:240;value:PowerSettings.displayMinutes;editable:true;enabled:PowerSettings.idleAvailable&&!PowerSettings.busy}}
  RowLayout{Layout.fillWidth:true;Label{text:root.t("Suspend after (minutes)","التعليق بعد (دقائق)")}SpinBox{id:suspendTimeout;from:0;to:240;value:PowerSettings.suspendMinutes;editable:true;enabled:PowerSettings.idleAvailable&&!PowerSettings.busy}}
  Label{text:PowerSettings.idleStatus;color:root.muted}
  HarborButton{text:root.t("Apply idle preferences","تطبيق تفضيلات الخمول");enabled:PowerSettings.idleAvailable&&!PowerSettings.busy;onClicked:PowerSettings.saveIdle(displayTimeout.value,suspendTimeout.value)}
 }
 Label{visible:PowerSettings.error.length>0;text:PowerSettings.error}
 Repeater{model:PowerSettings.batteries;delegate:Group{
  id:battery;required property var modelData
  Label{text:battery.modelData.name+" · "+battery.modelData.capacity+"% · "+battery.modelData.status;font.bold:true}
  Label{visible:!battery.modelData.canLimit;text:root.t("This battery does not expose charge limits to the operating system.","لا تتيح هذه البطارية حدود الشحن لنظام التشغيل.");color:root.muted}
  ColumnLayout{Layout.fillWidth:true;visible:battery.modelData.canLimit
   RowLayout{Layout.fillWidth:true;visible:battery.modelData.canStart;Label{text:root.t("Start charging below (%)","بدء الشحن دون (%)")}SpinBox{id:start;from:0;to:99;value:Math.max(0,battery.modelData.start);editable:true;enabled:!PowerSettings.busy}}
   RowLayout{Layout.fillWidth:true;Label{text:root.t("Stop charging at (%)","إيقاف الشحن عند (%)")}SpinBox{id:end;from:1;to:100;value:battery.modelData.end;editable:true;enabled:!PowerSettings.busy}}
   Label{text:root.t("Administrator authentication is required. Hardware may round values; limits may reset after restarting, depending on the battery driver.","تتطلب العملية مصادقة المسؤول. قد يقرّب الجهاز القيم، وقد تعود الحدود بعد إعادة التشغيل حسب برنامج تشغيل البطارية.");color:root.muted}
   HarborButton{text:root.t("Apply charge limits","تطبيق حدود الشحن");enabled:!PowerSettings.busy&&(!battery.modelData.canStart||start.value<end.value);onClicked:PowerSettings.setChargeLimits(battery.modelData.name,battery.modelData.canStart?start.value:-1,end.value)}
  }
 }}
 Label{visible:PowerSettings.batteries.length===0;text:root.t("No system battery detected.","لم يتم اكتشاف بطارية للنظام.");color:root.muted}
}
