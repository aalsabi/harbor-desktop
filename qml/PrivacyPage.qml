import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout{
 id:root;spacing:16;property var pageState:UserSettings.state.privacy||({apps:[],available:false});property var permissions:UserSettings.state.permission||({})
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 Component.onCompleted:UserSettings.request("privacy")
 NativePrivacySection{Layout.fillWidth:true}
 HarborLabel{text:root.t("Flatpak permissions","أذونات Flatpak");font.bold:true;font.pixelSize:17}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.t("Manage permissions for sandboxed Flatpak applications. Ordinary Debian applications do not have equivalent per-app isolation. Restart the affected application after changes.","إدارة أذونات تطبيقات Flatpak المعزولة. تطبيقات Debian العادية لا تملك عزلًا مماثلًا لكل تطبيق. أعد تشغيل التطبيق بعد التغيير.")}
 ComboBox{id:apps;Layout.fillWidth:true;model:root.pageState.apps;textRole:"name";valueRole:"id";Accessible.name:root.t("Application","التطبيق");onActivated:UserSettings.request("permission",{id:currentValue})}
 HarborButton{text:root.t("View permissions","عرض الأذونات");enabled:!UserSettings.busy&&apps.currentIndex>=0;onClicked:UserSettings.request("permission",{id:apps.currentValue})}
 Repeater{model:[{key:"network",en:"Network access",ar:"الوصول للشبكة"},{key:"audio",en:"Audio and microphone",ar:"الصوت والميكروفون"},{key:"home",en:"Broad home-folder access",ar:"الوصول العام للمجلد الشخصي"}];delegate:RowLayout{required property var modelData;Layout.fillWidth:true;HarborLabel{Layout.fillWidth:true;text:root.t(modelData.en,modelData.ar)}Switch{checked:root.permissions[modelData.key]===true;enabled:!UserSettings.busy&&root.permissions.id===apps.currentValue;Accessible.name:root.t(modelData.en,modelData.ar);onClicked:{UserSettings.request("permission-set",{id:apps.currentValue,key:modelData.key,value:checked});checked=Qt.binding(function(){return root.permissions[modelData.key]===true})}}}}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.t("Access to individual folders or files previously granted through file dialogs may remain. The permission details below show the effective Flatpak configuration.","قد يبقى الوصول إلى ملفات أو مجلدات منفردة مُنحت سابقًا من نوافذ اختيار الملفات. التفاصيل التالية تعرض إعدادات Flatpak الفعلية.")}
 TextArea{Layout.fillWidth:true;readOnly:true;wrapMode:TextEdit.WrapAnywhere;text:root.permissions.raw||"";visible:text.length>0;Accessible.name:root.t("Permission details","تفاصيل الأذونات")}
 HarborLabel{visible:!root.pageState.available;text:root.t("Flatpak is not installed.","Flatpak غير مثبت.")}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:UserSettings.error}
}
