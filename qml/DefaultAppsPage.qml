import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id:root;objectName:"defaultsPage";spacing:14
 property color ink:Prefs.dark?"#eeeef0":"#252527"
 property color muted:Prefs.dark?"#a6a6ad":"#76767c"
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 function title(key){return ({browser:t("Web browser","متصفح الويب"),mail:t("Email","البريد الإلكتروني"),files:t("File manager","مدير الملفات"),pdf:t("PDF documents","مستندات PDF"),text:t("Text files","الملفات النصية"),images:t("Images","الصور")})[key]||key}
 Component.onCompleted:DefaultApps.refresh()
 Text{Layout.fillWidth:true;wrapMode:Text.WordWrap;color:root.muted;font.pixelSize:13;text:root.t("Choose which applications open links and common file types for your account.","اختر التطبيقات التي تفتح الروابط وأنواع الملفات الشائعة لحسابك.")}
 Repeater{model:DefaultApps.roles;delegate:Rectangle{
  required property var modelData
  Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7"
  ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:8
   Text{text:root.title(modelData.key);font.pixelSize:14;font.bold:true;color:root.ink}
   Text{Layout.fillWidth:true;wrapMode:Text.Wrap;color:root.muted;font.pixelSize:12;text:modelData.mixed?root.t("Different applications are used for these types.","تُستخدم تطبيقات مختلفة لهذه الأنواع."):modelData.currentName?root.t("Current: ","الحالي: ")+modelData.currentName:root.t("No default application is set.","لم يُعيّن تطبيق افتراضي.")}
   ComboBox{
    id:select;Layout.fillWidth:true;model:modelData.apps;textRole:"name";valueRole:"id";currentIndex:modelData.currentIndex;enabled:modelData.apps.length>0
    displayText:currentIndex>=0?currentText:root.t("Choose an application","اختر تطبيقًا")
    Accessible.name:root.title(modelData.key)
    onActivated:{if(!DefaultApps.setDefault(modelData.key,currentValue))DefaultApps.refresh();currentIndex=Qt.binding(function(){return modelData.currentIndex})}
    contentItem:Text{text:select.displayText;color:root.ink;font.pixelSize:13;verticalAlignment:Text.AlignVCenter;elide:Text.ElideRight;leftPadding:10;rightPadding:28}
    background:Rectangle{radius:6;color:Prefs.dark?"#45454b":"#fafafa";border.color:select.activeFocus?Prefs.accent:(Prefs.dark?"#55555b":"#d9d9df")}
   }
   Text{visible:modelData.apps.length===0;Layout.fillWidth:true;wrapMode:Text.Wrap;color:root.muted;font.pixelSize:12;text:root.t("No installed application supports all of these types.","لا يوجد تطبيق مثبت يدعم جميع هذه الأنواع.")}
   Text{Layout.fillWidth:true;wrapMode:Text.Wrap;color:root.muted;font.pixelSize:11;text:modelData.types}
  }
 }}
 Text{Layout.fillWidth:true;visible:text.length>0;text:DefaultApps.message;wrapMode:Text.WordWrap;color:root.ink;font.pixelSize:13}
 HarborButton{text:root.t("Refresh applications","تحديث التطبيقات");onClicked:DefaultApps.refresh()}
}
