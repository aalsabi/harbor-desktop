import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout{
 id:root;objectName:"updatesPage";spacing:14
 property color ink:Prefs.dark?"#eeeef0":"#252527"
 property color muted:Prefs.dark?"#a6a6ad":"#76767c"
 property var selectedIds:[]
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 function choose(id,value){let ids=selectedIds.slice();let i=ids.indexOf(id);if(value&&i<0)ids.push(id);else if(!value&&i>=0)ids.splice(i,1);selectedIds=ids;SoftwareUpdate.discardPreview();confirmInstall.checked=false}
 function action(info){return info===13?t("Remove","إزالة"):info===15?t("Replace","استبدال"):info===12?t("Install","تثبيت"):info===11?t("Update","تحديث"):t("Change","تغيير")}
 component Card:Rectangle{default property alias contents:body.data;Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7";ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:10}}
 component Caption:Text{Layout.fillWidth:true;color:root.ink;font.pixelSize:13;wrapMode:Text.WordWrap}
 Caption{text:root.t("Review updates from your distribution. Installation starts only after a dependency preview and your confirmation.","راجع تحديثات توزيعتك. يبدأ التثبيت بعد معاينة التغييرات وتأكيدك فقط.");color:root.muted}
 RowLayout{Layout.fillWidth:true
  HarborButton{text:root.t("Check updates","البحث عن تحديثات");enabled:!SoftwareUpdate.busy;onClicked:{root.selectedIds=[];confirmInstall.checked=false;SoftwareUpdate.checkUpdates(false)}}
  HarborButton{text:root.t("Refresh package information","تحديث معلومات الحزم");enabled:!SoftwareUpdate.busy;onClicked:{root.selectedIds=[];confirmInstall.checked=false;SoftwareUpdate.checkUpdates(true)}}
 }
 Caption{visible:SoftwareUpdate.status.length>0;text:SoftwareUpdate.status}
 ProgressBar{Layout.fillWidth:true;visible:SoftwareUpdate.busy;from:0;to:100;value:SoftwareUpdate.percentage;indeterminate:SoftwareUpdate.percentage<0}
 Card{visible:SoftwareUpdate.packages.length>0
  RowLayout{Layout.fillWidth:true;Caption{text:root.t("Available updates","التحديثات المتاحة");font.bold:true}HarborButton{text:root.t("Select all","تحديد الكل");enabled:!SoftwareUpdate.busy;onClicked:{root.selectedIds=SoftwareUpdate.packages.filter(p=>!p.blocked).map(p=>p.id);SoftwareUpdate.discardPreview();confirmInstall.checked=false}}}
  Repeater{model:SoftwareUpdate.packages;delegate:ColumnLayout{required property var modelData;Layout.fillWidth:true;spacing:3
   CheckBox{id:choice;Layout.fillWidth:true;enabled:!SoftwareUpdate.busy&&!modelData.blocked;checked:root.selectedIds.indexOf(modelData.id)>=0;onClicked:root.choose(modelData.id,checked);text:modelData.name+" · "+modelData.version+(modelData.security?root.t(" · Security update"," · تحديث أمني"):"")+(modelData.blocked?root.t(" · Blocked"," · محظور"):"");contentItem:Text{text:choice.text;color:root.ink;leftPadding:choice.indicator.width+8;wrapMode:Text.WordWrap;font.pixelSize:13}}
   Caption{text:modelData.summary;color:root.muted;font.pixelSize:12}
  }}
  HarborButton{text:root.t("Review selected updates","مراجعة التحديثات المحددة");prominent:true;enabled:!SoftwareUpdate.busy&&root.selectedIds.length>0;onClicked:{confirmInstall.checked=false;SoftwareUpdate.prepare(root.selectedIds)}}
 }
 Card{visible:SoftwareUpdate.preview.length>0
  Caption{text:root.t("Dependency preview","معاينة تغييرات الحزم");font.bold:true}
  Caption{text:root.t("This includes dependencies and any packages that will be removed or replaced. Package availability can change before installation.","تشمل هذه القائمة الاعتماديات والحزم التي ستُزال أو تُستبدل. قد يتغير توفر الحزم قبل التثبيت.");color:root.muted}
  Repeater{model:SoftwareUpdate.preview;delegate:Caption{required property var modelData;text:root.action(modelData.info)+" · "+modelData.name+" · "+modelData.version}}
  CheckBox{id:confirmInstall;Layout.fillWidth:true;visible:SoftwareUpdate.readyToInstall;text:root.t("I reviewed these changes and want to install them","راجعت هذه التغييرات وأريد تثبيتها");contentItem:Text{text:confirmInstall.text;color:root.ink;leftPadding:confirmInstall.indicator.width+8;wrapMode:Text.WordWrap;font.pixelSize:13}}
  HarborButton{text:root.t("Install trusted updates","تثبيت التحديثات الموثوقة");prominent:true;enabled:!SoftwareUpdate.busy&&SoftwareUpdate.readyToInstall&&confirmInstall.checked;onClicked:{SoftwareUpdate.install(true);confirmInstall.checked=false}}
 }
 Caption{visible:SoftwareUpdate.error.length>0;text:SoftwareUpdate.error}
 Caption{visible:SoftwareUpdate.restart.length>0;text:SoftwareUpdate.restart;font.bold:true}
 Caption{text:root.t("Unsigned packages, new repository keys, and license agreements are never accepted automatically.","لا يتم قبول الحزم غير الموقعة أو مفاتيح المستودعات الجديدة أو اتفاقيات الترخيص تلقائيًا.");color:root.muted}
}
