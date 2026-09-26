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
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 Component.onCompleted:Accounts.refresh()
 component Card:Rectangle{default property alias contents:body.data;Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7";ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:10}}
 component Caption:Text{Layout.fillWidth:true;color:root.ink;font.pixelSize:13;wrapMode:Text.WordWrap}
 component Choice:ComboBox{Layout.fillWidth:true;contentItem:Text{text:parent.displayText;color:root.ink;verticalAlignment:Text.AlignVCenter;leftPadding:10;rightPadding:24;elide:Text.ElideRight}background:Rectangle{radius:6;color:Prefs.dark?"#45454b":"#fafafa";border.color:Prefs.dark?"#55555b":"#d9d9df"}}
 RowLayout{Layout.fillWidth:true;Caption{text:root.t("Manage local accounts. Your system asks for authorization when needed.","إدارة الحسابات المحلية. سيطلب النظام المصادقة عند الحاجة.");color:root.muted}HarborButton{text:root.t("Refresh","تحديث");enabled:!Accounts.busy;onClicked:Accounts.refresh()}}
 Card{
  Caption{text:root.t("Accounts","الحسابات");font.bold:true}
  Repeater{model:Accounts.users;delegate:HarborButton{required property var modelData;Layout.fillWidth:true;text:(modelData.RealName||modelData.UserName)+" · "+modelData.UserName+(modelData.AccountType===1?root.t(" · Administrator"," · مسؤول"):"")+(modelData.isCurrent?root.t(" · You"," · أنت"):"");prominent:root.selectedPath===modelData.path;onClicked:{root.selectedPath=modelData.path;realName.text=modelData.RealName;newPassword.clear();repeatPassword.clear();root.confirmation="";deleteHome.checked=false}}}
  Caption{visible:!Accounts.busy&&Accounts.users.length===0;text:root.t("No accounts were returned by AccountsService.","لم تُرجع خدمة الحسابات أي حسابات.");color:root.muted}
 }
 Card{visible:root.selected!==null
  Caption{text:root.selected?(root.selected.RealName||root.selected.UserName):"";font.bold:true}
  HarborField{id:realName;Layout.fillWidth:true;placeholderText:root.t("Display name","اسم العرض");Accessible.name:placeholderText}
  HarborButton{text:root.t("Save name","حفظ الاسم");enabled:!Accounts.busy;onClicked:Accounts.setRealName(root.selectedPath,realName.text)}
  RowLayout{Layout.fillWidth:true
   Caption{text:root.selected&&root.selected.AccountType===1?root.t("Administrator account","حساب مسؤول"):root.t("Standard account","حساب عادي")}
   HarborButton{text:root.selected&&root.selected.AccountType===1?root.t("Make standard","تحويل إلى عادي"):root.t("Make administrator","تحويل إلى مسؤول");enabled:!Accounts.busy&&root.selected!==null&&(!root.selected.AccountType||root.selected.canRemove);onClicked:root.confirmation="type"}
  }
  Caption{text:root.t("New password (at least 8 characters)","كلمة مرور جديدة (8 أحرف على الأقل)")}
  HarborField{id:newPassword;Layout.fillWidth:true;echoMode:TextInput.Password;inputMethodHints:Qt.ImhSensitiveData|Qt.ImhNoPredictiveText;placeholderText:root.t("New password","كلمة المرور الجديدة");Accessible.name:placeholderText}
  HarborField{id:repeatPassword;Layout.fillWidth:true;echoMode:TextInput.Password;inputMethodHints:Qt.ImhSensitiveData|Qt.ImhNoPredictiveText;placeholderText:root.t("Repeat password","تكرار كلمة المرور");Accessible.name:placeholderText}
  Caption{text:root.t("Setting a password also unlocks this account.","تعيين كلمة مرور يفتح قفل هذا الحساب أيضًا.");color:root.muted}
  HarborButton{text:root.t("Set password","تعيين كلمة المرور");enabled:!Accounts.busy&&newPassword.text.length>=8&&newPassword.text===repeatPassword.text;onClicked:{Accounts.setPassword(root.selectedPath,newPassword.text);newPassword.clear();repeatPassword.clear()}}
  HarborButton{text:root.t("Delete account…","حذف الحساب…");enabled:!Accounts.busy&&root.selected!==null&&root.selected.canRemove;onClicked:{root.confirmation="delete";deleteHome.checked=false}}
  ColumnLayout{visible:root.confirmation.length>0;Layout.fillWidth:true
   Caption{text:root.confirmation==="delete"?root.t("Delete this account? This cannot be undone.","حذف هذا الحساب؟ لا يمكن التراجع عن هذا الإجراء."):root.t("Change this account's administrator privileges?","تغيير صلاحيات المسؤول لهذا الحساب؟")}
   CheckBox{id:deleteHome;visible:root.confirmation==="delete";text:root.t("Also permanently delete the home folder and its files","حذف المجلد الشخصي وملفاته نهائيًا أيضًا");contentItem:Text{text:deleteHome.text;color:root.ink;leftPadding:deleteHome.indicator.width+8;wrapMode:Text.WordWrap}Layout.fillWidth:true}
   RowLayout{HarborButton{text:root.t("Cancel","إلغاء");onClicked:root.confirmation=""}HarborButton{text:root.t("Confirm","تأكيد");enabled:!Accounts.busy;onClicked:{if(root.confirmation==="delete")Accounts.deleteUser(root.selectedPath,deleteHome.checked,true);else Accounts.setAccountType(root.selectedPath,root.selected.AccountType===1?0:1,true);root.confirmation=""}}}
  }
 }
 Card{
  Caption{text:root.t("Create account","إنشاء حساب");font.bold:true}
  HarborField{id:username;Layout.fillWidth:true;placeholderText:root.t("Account name (lowercase letters)","اسم الحساب (أحرف إنجليزية صغيرة)");Accessible.name:placeholderText}
  HarborField{id:fullName;Layout.fillWidth:true;placeholderText:root.t("Full name","الاسم الكامل");Accessible.name:placeholderText}
  Choice{id:accountType;model:[root.t("Standard","عادي"),root.t("Administrator","مسؤول")];Accessible.name:root.t("Account type","نوع الحساب")}
  HarborField{id:createPassword;Layout.fillWidth:true;echoMode:TextInput.Password;inputMethodHints:Qt.ImhSensitiveData|Qt.ImhNoPredictiveText;placeholderText:root.t("Password (8+ characters)","كلمة المرور (8 أحرف أو أكثر)");Accessible.name:placeholderText}
  HarborField{id:createRepeat;Layout.fillWidth:true;echoMode:TextInput.Password;inputMethodHints:Qt.ImhSensitiveData|Qt.ImhNoPredictiveText;placeholderText:root.t("Repeat password","تكرار كلمة المرور");Accessible.name:placeholderText}
  HarborButton{text:root.t("Create account","إنشاء حساب");prominent:true;enabled:!Accounts.busy&&username.text.length>0&&fullName.text.length>0&&createPassword.text.length>=8&&createPassword.text===createRepeat.text;onClicked:{Accounts.createUser(username.text,fullName.text,accountType.currentIndex,createPassword.text);createPassword.clear();createRepeat.clear()}}
 }
 Caption{visible:Accounts.busy;text:root.t("Working… Complete the system authorization prompt if requested.","جارٍ التنفيذ… أكمل طلب مصادقة النظام إن ظهر.");color:root.muted}
 Caption{visible:Accounts.error.length>0;text:Accounts.error}
}
