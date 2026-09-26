import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout{
 id:root;spacing:16;property var pageState:UserSettings.state.startup||({entries:[],applications:[]})
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 Component.onCompleted:UserSettings.request("startup")
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.t("Choose which applications start when you log in to Harbor. Changes apply at your next login.","اختر التطبيقات التي تعمل عند دخول Harbor. تسري التغييرات عند الدخول التالي.")}
 Repeater{model:root.pageState.entries;delegate:RowLayout{required property var modelData;Layout.fillWidth:true;HarborLabel{Layout.fillWidth:true;text:modelData.name;textFormat:Text.PlainText;elide:Text.ElideRight}Switch{checked:modelData.enabled;enabled:!UserSettings.busy;Accessible.name:modelData.name;onClicked:{UserSettings.request("startup-set",{id:modelData.id,enabled:checked});checked=Qt.binding(function(){return modelData.enabled})}}}}
 ComboBox{id:choice;Layout.fillWidth:true;model:root.pageState.applications;textRole:"name";valueRole:"id";Accessible.name:root.t("Application to add","التطبيق المراد إضافته")}
 RowLayout{HarborButton{text:root.t("Add startup application","إضافة تطبيق لبدء التشغيل");enabled:!UserSettings.busy&&choice.currentIndex>=0;onClicked:UserSettings.request("startup-add",{id:choice.currentValue})}HarborButton{text:root.t("Refresh","تحديث");enabled:!UserSettings.busy;onClicked:UserSettings.request("startup")}}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:UserSettings.error}
}
