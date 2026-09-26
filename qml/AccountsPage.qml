import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id:root;objectName:"accountsPage";spacing:14
 property color ink:Prefs.dark?"#eeeef0":"#252527"
 property color muted:Prefs.dark?"#a6a6ad":"#76767c"
 property string selectedPath:""
 property var selected:Accounts.users.find(u=>u.path===selectedPath)||null
 property string confirmation:""
 Component.onCompleted:Accounts.refresh()
 component Card:Rectangle{default property alias contents:body.data;Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7";ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:10}}
 component Caption:Text{Layout.fillWidth:true;color:root.ink;font.pixelSize:13;wrapMode:Text.WordWrap}
 component Choice:ComboBox{Layout.fillWidth:true;contentItem:Text{text:parent.displayText;color:root.ink;verticalAlignment:Text.AlignVCenter;leftPadding:10;rightPadding:24;elide:Text.ElideRight}background:Rectangle{radius:6;color:Prefs.dark?"#45454b":"#fafafa";border.color:Prefs.dark?"#55555b":"#d9d9df"}}
 RowLayout{Layout.fillWidth:true;Caption{text:qsTr("Manage local accounts. Your system asks for authorization when needed.");color:root.muted}HarborButton{text:qsTr("Refresh");enabled:!Accounts.busy;onClicked:Accounts.refresh()}}
 Card{
  Caption{text:qsTr("Accounts");font.bold:true}
  Repeater{model:Accounts.users;delegate:HarborButton{required property var modelData;Layout.fillWidth:true;text:(modelData.RealName||modelData.UserName)+" · "+modelData.UserName+(modelData.AccountType===1?qsTr(" · Administrator"):"")+(modelData.isCurrent?qsTr(" · You"):"");prominent:root.selectedPath===modelData.path;onClicked:{root.selectedPath=modelData.path;realName.text=modelData.RealName;newPassword.clear();repeatPassword.clear();root.confirmation="";deleteHome.checked=false}}}
  Caption{visible:!Accounts.busy&&Accounts.users.length===0;text:qsTr("No accounts were returned by AccountsService.");color:root.muted}
 }
 Card{visible:root.selected!==null
  Caption{text:root.selected?(root.selected.RealName||root.selected.UserName):"";font.bold:true}
  HarborField{id:realName;Layout.fillWidth:true;placeholderText:qsTr("Display name");Accessible.name:placeholderText}
  HarborButton{text:qsTr("Save name");enabled:!Accounts.busy;onClicked:Accounts.setRealName(root.selectedPath,realName.text)}
  RowLayout{Layout.fillWidth:true
   Caption{text:root.selected&&root.selected.AccountType===1?qsTr("Administrator account"):qsTr("Standard account")}
   HarborButton{text:root.selected&&root.selected.AccountType===1?qsTr("Make standard"):qsTr("Make administrator");enabled:!Accounts.busy&&root.selected!==null&&(!root.selected.AccountType||root.selected.canRemove);onClicked:root.confirmation="type"}
  }
  Caption{text:qsTr("New password (at least 8 characters)")}
  HarborField{id:newPassword;Layout.fillWidth:true;echoMode:TextInput.Password;inputMethodHints:Qt.ImhSensitiveData|Qt.ImhNoPredictiveText;placeholderText:qsTr("New password");Accessible.name:placeholderText}
  HarborField{id:repeatPassword;Layout.fillWidth:true;echoMode:TextInput.Password;inputMethodHints:Qt.ImhSensitiveData|Qt.ImhNoPredictiveText;placeholderText:qsTr("Repeat password");Accessible.name:placeholderText}
  Caption{text:qsTr("Setting a password also unlocks this account.");color:root.muted}
  HarborButton{text:qsTr("Set password");enabled:!Accounts.busy&&newPassword.text.length>=8&&newPassword.text===repeatPassword.text;onClicked:{Accounts.setPassword(root.selectedPath,newPassword.text);newPassword.clear();repeatPassword.clear()}}
  HarborButton{text:qsTr("Delete account…");enabled:!Accounts.busy&&root.selected!==null&&root.selected.canRemove;onClicked:{root.confirmation="delete";deleteHome.checked=false}}
  ColumnLayout{visible:root.confirmation.length>0;Layout.fillWidth:true
   Caption{text:root.confirmation==="delete"?qsTr("Delete this account? This cannot be undone."):qsTr("Change this account's administrator privileges?")}
   CheckBox{id:deleteHome;visible:root.confirmation==="delete";text:qsTr("Also permanently delete the home folder and its files");contentItem:Text{text:deleteHome.text;color:root.ink;leftPadding:deleteHome.indicator.width+8;wrapMode:Text.WordWrap}Layout.fillWidth:true}
   RowLayout{HarborButton{text:qsTr("Cancel");onClicked:root.confirmation=""}HarborButton{text:qsTr("Confirm");enabled:!Accounts.busy;onClicked:{if(root.confirmation==="delete")Accounts.deleteUser(root.selectedPath,deleteHome.checked,true);else Accounts.setAccountType(root.selectedPath,root.selected.AccountType===1?0:1,true);root.confirmation=""}}}
  }
 }
 Card{
  Caption{text:qsTr("Create account");font.bold:true}
  HarborField{id:username;Layout.fillWidth:true;placeholderText:qsTr("Account name (lowercase letters)");Accessible.name:placeholderText}
  HarborField{id:fullName;Layout.fillWidth:true;placeholderText:qsTr("Full name");Accessible.name:placeholderText}
  Choice{id:accountType;model:[qsTr("Standard"),qsTr("Administrator")];Accessible.name:qsTr("Account type")}
  HarborField{id:createPassword;Layout.fillWidth:true;echoMode:TextInput.Password;inputMethodHints:Qt.ImhSensitiveData|Qt.ImhNoPredictiveText;placeholderText:qsTr("Password (8+ characters)");Accessible.name:placeholderText}
  HarborField{id:createRepeat;Layout.fillWidth:true;echoMode:TextInput.Password;inputMethodHints:Qt.ImhSensitiveData|Qt.ImhNoPredictiveText;placeholderText:qsTr("Repeat password");Accessible.name:placeholderText}
  HarborButton{text:qsTr("Create account");prominent:true;enabled:!Accounts.busy&&username.text.length>0&&fullName.text.length>0&&createPassword.text.length>=8&&createPassword.text===createRepeat.text;onClicked:{Accounts.createUser(username.text,fullName.text,accountType.currentIndex,createPassword.text);createPassword.clear();createRepeat.clear()}}
 }
 Caption{visible:Accounts.busy;text:qsTr("Working… Complete the system authorization prompt if requested.");color:root.muted}
 Caption{visible:Accounts.error.length>0;text:Accounts.error}
}
