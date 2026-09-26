import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout{
 id:root;spacing:16;property var pageState:UserSettings.state.privacy||({apps:[],available:false});property var permissions:UserSettings.state.permission||({})
 Component.onCompleted:UserSettings.request("privacy")
 NativePrivacySection{Layout.fillWidth:true}
 HarborLabel{text:qsTr("Flatpak permissions");font.bold:true;font.pixelSize:17}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:qsTr("Manage permissions for sandboxed Flatpak applications. Ordinary Debian applications do not have equivalent per-app isolation. Restart the affected application after changes.")}
 ComboBox{id:apps;Layout.fillWidth:true;model:root.pageState.apps;textRole:"name";valueRole:"id";Accessible.name:qsTr("Application");onActivated:UserSettings.request("permission",{id:currentValue})}
 HarborButton{text:qsTr("View permissions");enabled:!UserSettings.busy&&apps.currentIndex>=0;onClicked:UserSettings.request("permission",{id:apps.currentValue})}
 Repeater{model:[{key:"network",en:QT_TR_NOOP("Network access")},{key:"audio",en:QT_TR_NOOP("Audio and microphone")},{key:"home",en:QT_TR_NOOP("Broad home-folder access")}];delegate:RowLayout{required property var modelData;Layout.fillWidth:true;HarborLabel{Layout.fillWidth:true;text:qsTr(modelData.en)}Switch{checked:root.permissions[modelData.key]===true;enabled:!UserSettings.busy&&root.permissions.id===apps.currentValue;Accessible.name:qsTr(modelData.en);onClicked:{UserSettings.request("permission-set",{id:apps.currentValue,key:modelData.key,value:checked});checked=Qt.binding(function(){return root.permissions[modelData.key]===true})}}}}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:qsTr("Access to individual folders or files previously granted through file dialogs may remain. The permission details below show the effective Flatpak configuration.")}
 TextArea{Layout.fillWidth:true;readOnly:true;wrapMode:TextEdit.WrapAnywhere;text:root.permissions.raw||"";visible:text.length>0;Accessible.name:qsTr("Permission details")}
 HarborLabel{visible:!root.pageState.available;text:qsTr("Flatpak is not installed.")}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:UserSettings.error}
}
