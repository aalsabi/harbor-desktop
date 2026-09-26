import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout{
 id:root;spacing:12
 property string editId:"";property var days:[1,2,3,4,5];property var allowed:[];property bool editing:false
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 function edit(p){editId=p.id||"";name.text=p.name||"";start.text=p.start||"09:00";end.text=p.end||"17:00";schedule.checked=!!p.scheduled;days=p.days||[1,2,3,4,5];allowed=p.allowed||[];editing=true}
 function toggleDay(day,on){let values=days.slice();if(on&&!values.includes(day))values.push(day);if(!on)values=values.filter(x=>x!==day);days=values}
 function toggleApp(id,on){let values=allowed.slice();if(on&&!values.includes(id))values.push(id);if(!on)values=values.filter(x=>x!==id);allowed=values}
 HarborLabel{text:root.t("Focus profiles","أوضاع التركيز");font.bold:true;font.pixelSize:17}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.t("Schedules follow local time. Overlapping schedules use the first profile below. Manual selection overrides schedules until you return to Automatic. History is retained; only allowed apps contribute notification attention.","تتبع الجداول الوقت المحلي. عند التداخل يُستخدم أول وضع أدناه. الاختيار اليدوي يتجاوز الجداول حتى العودة إلى تلقائي. يبقى السجل وتظهر مؤشرات التنبيه للتطبيقات المسموحة فقط.")}
 ComboBox{Accessible.name:root.t("Focus mode","وضع التركيز");Layout.fillWidth:true;model:[{id:"auto",name:root.t("Automatic schedules","الجداول تلقائيًا")},{id:"off",name:root.t("Focus off","إيقاف التركيز")}].concat(NotificationPrefs.profiles);textRole:"name";valueRole:"id";currentIndex:model.findIndex(x=>x.id===NotificationPrefs.focusMode);onActivated:{NotificationPrefs.focusMode=currentValue;currentIndex=Qt.binding(()=>model.findIndex(x=>x.id===NotificationPrefs.focusMode))}}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.t("Active: ","النشط: ")+(NotificationPrefs.activeProfile.name||root.t("None","لا يوجد"))}
 HarborLabel{visible:NotificationPrefs.doNotDisturb;Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.t("Do Not Disturb is on and silences attention from all apps, including this profile’s allowed apps.","عدم الإزعاج مفعّل ويخفي مؤشرات التنبيه من جميع التطبيقات، بما فيها التطبيقات المسموحة في هذا الوضع.")}
 Repeater{model:NotificationPrefs.profiles;delegate:RowLayout{required property var modelData;Layout.fillWidth:true;HarborLabel{Layout.fillWidth:true;text:modelData.name;textFormat:Text.PlainText;elide:Text.ElideRight}HarborButton{text:root.t("Edit","تعديل");onClicked:root.edit(modelData)}HarborButton{text:root.t("Remove","إزالة");onClicked:{NotificationPrefs.removeProfile(modelData.id);if(root.editId===modelData.id)root.editing=false}}}}
 HarborButton{text:root.t("Add Focus profile","إضافة وضع تركيز");onClicked:root.edit({})}
 ColumnLayout{visible:root.editing;Layout.fillWidth:true;spacing:10
  HarborField{id:name;objectName:"focus-profile-name";Layout.fillWidth:true;placeholderText:root.t("Profile name","اسم الوضع");maximumLength:80;Accessible.name:placeholderText}
  CheckBox{id:schedule;text:root.t("Enable weekly schedule","تفعيل جدول أسبوعي")}
  Flow{Layout.fillWidth:true;spacing:4;Repeater{model:["Mon","Tue","Wed","Thu","Fri","Sat","Sun"];delegate:CheckBox{required property int index;required property string modelData;text:root.t(modelData,["إثنين","ثلاثاء","أربعاء","خميس","جمعة","سبت","أحد"][index]);checked:root.days.includes(index+1);onClicked:{root.toggleDay(index+1,checked);checked=Qt.binding(()=>root.days.includes(index+1))}}}}
  RowLayout{Layout.fillWidth:true;HarborField{id:start;Layout.fillWidth:true;placeholderText:"09:00";maximumLength:5;Accessible.name:root.t("Start time HH:mm","وقت البدء HH:mm")}HarborLabel{text:"—"}HarborField{id:end;Layout.fillWidth:true;placeholderText:"17:00";maximumLength:5;Accessible.name:root.t("End time HH:mm","وقت الانتهاء HH:mm")}}
  HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.t("For overnight schedules, choose the day on which the schedule starts. Choose applications allowed to show attention:","للجدول الليلي اختر يوم بداية الجدول. اختر التطبيقات المسموح لها بإظهار التنبيه:")}
  Repeater{model:NotificationPrefs.applications;delegate:CheckBox{required property var modelData;Layout.fillWidth:true;text:modelData.name;checked:root.allowed.includes(modelData.id);onClicked:{root.toggleApp(modelData.id,checked);checked=Qt.binding(()=>root.allowed.includes(modelData.id))}}}
  RowLayout{HarborButton{objectName:"save-focus-profile";text:root.t("Save profile","حفظ الوضع");onClicked:if(NotificationPrefs.saveProfile({id:root.editId,name:name.text,scheduled:schedule.checked,days:root.days,start:start.text,end:end.text,allowed:root.allowed}))root.editing=false}HarborButton{text:root.t("Cancel","إلغاء");onClicked:root.editing=false}}
 }
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:NotificationPrefs.error}
}
