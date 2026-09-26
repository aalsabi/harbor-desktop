import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout{
 id:root;spacing:12
 property string editId:"";property var days:[1,2,3,4,5];property var allowed:[];property bool editing:false
 function edit(p){editId=p.id||"";name.text=p.name||"";start.text=p.start||"09:00";end.text=p.end||"17:00";schedule.checked=!!p.scheduled;days=p.days||[1,2,3,4,5];allowed=p.allowed||[];editing=true}
 function toggleDay(day,on){let values=days.slice();if(on&&!values.includes(day))values.push(day);if(!on)values=values.filter(x=>x!==day);days=values}
 function toggleApp(id,on){let values=allowed.slice();if(on&&!values.includes(id))values.push(id);if(!on)values=values.filter(x=>x!==id);allowed=values}
 HarborLabel{text:qsTr("Focus profiles");font.bold:true;font.pixelSize:17}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:qsTr("Schedules follow local time. Overlapping schedules use the first profile below. Manual selection overrides schedules until you return to Automatic. History is retained; only allowed apps contribute notification attention.")}
 ComboBox{Accessible.name:qsTr("Focus mode");Layout.fillWidth:true;model:[{id:"auto",name:qsTr("Automatic schedules")},{id:"off",name:qsTr("Focus off")}].concat(NotificationPrefs.profiles);textRole:"name";valueRole:"id";currentIndex:model.findIndex(x=>x.id===NotificationPrefs.focusMode);onActivated:{NotificationPrefs.focusMode=currentValue;currentIndex=Qt.binding(()=>model.findIndex(x=>x.id===NotificationPrefs.focusMode))}}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:qsTr("Active: ")+(NotificationPrefs.activeProfile.name||qsTr("None"))}
 HarborLabel{visible:NotificationPrefs.doNotDisturb;Layout.fillWidth:true;wrapMode:Text.Wrap;text:qsTr("Do Not Disturb is on and silences attention from all apps, including this profile’s allowed apps.")}
 Repeater{model:NotificationPrefs.profiles;delegate:RowLayout{required property var modelData;Layout.fillWidth:true;HarborLabel{Layout.fillWidth:true;text:modelData.name;textFormat:Text.PlainText;elide:Text.ElideRight}HarborButton{text:qsTr("Edit");onClicked:root.edit(modelData)}HarborButton{text:qsTr("Remove");onClicked:{NotificationPrefs.removeProfile(modelData.id);if(root.editId===modelData.id)root.editing=false}}}}
 HarborButton{text:qsTr("Add Focus profile");onClicked:root.edit({})}
 ColumnLayout{visible:root.editing;Layout.fillWidth:true;spacing:10
  HarborField{id:name;objectName:"focus-profile-name";Layout.fillWidth:true;placeholderText:qsTr("Profile name");maximumLength:80;Accessible.name:placeholderText}
  CheckBox{id:schedule;text:qsTr("Enable weekly schedule")}
  Flow{Layout.fillWidth:true;spacing:4;Repeater{model:[QT_TR_NOOP("Mon"),QT_TR_NOOP("Tue"),QT_TR_NOOP("Wed"),QT_TR_NOOP("Thu"),QT_TR_NOOP("Fri"),QT_TR_NOOP("Sat"),QT_TR_NOOP("Sun")];delegate:CheckBox{required property int index;required property string modelData;text:qsTr(modelData);checked:root.days.includes(index+1);onClicked:{root.toggleDay(index+1,checked);checked=Qt.binding(()=>root.days.includes(index+1))}}}}
  RowLayout{Layout.fillWidth:true;HarborField{id:start;Layout.fillWidth:true;placeholderText:"09:00";maximumLength:5;Accessible.name:qsTr("Start time HH:mm")}HarborLabel{text:"—"}HarborField{id:end;Layout.fillWidth:true;placeholderText:"17:00";maximumLength:5;Accessible.name:qsTr("End time HH:mm")}}
  HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:qsTr("For overnight schedules, choose the day on which the schedule starts. Choose applications allowed to show attention:")}
  Repeater{model:NotificationPrefs.applications;delegate:CheckBox{required property var modelData;Layout.fillWidth:true;text:modelData.name;checked:root.allowed.includes(modelData.id);onClicked:{root.toggleApp(modelData.id,checked);checked=Qt.binding(()=>root.allowed.includes(modelData.id))}}}
  RowLayout{HarborButton{objectName:"save-focus-profile";text:qsTr("Save profile");onClicked:if(NotificationPrefs.saveProfile({id:root.editId,name:name.text,scheduled:schedule.checked,days:root.days,start:start.text,end:end.text,allowed:root.allowed}))root.editing=false}HarborButton{text:qsTr("Cancel");onClicked:root.editing=false}}
 }
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:NotificationPrefs.error}
}
