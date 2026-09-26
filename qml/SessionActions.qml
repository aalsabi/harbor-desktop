import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
RowLayout {
 id:root;spacing:8
 property string pending:""
 property string caption:""
 function ask(action,label){pending=action;caption=label;confirm.open()}
 HarborButton{objectName:"session-logout";text:qsTr("Log Out…");enabled:!!System.state.harborSession;onClicked:root.ask("logout",text)}
 HarborButton{objectName:"session-restart";text:qsTr("Restart…");enabled:!!System.state.canReboot;onClicked:root.ask("reboot",text)}
 HarborButton{objectName:"session-poweroff";text:qsTr("Shut Down…");enabled:!!System.state.canPowerOff;onClicked:root.ask("poweroff",text)}
 Dialog{id:confirm;objectName:"session-confirm";parent:Overlay.overlay;anchors.centerIn:parent;modal:true;title:root.caption;standardButtons:Dialog.Ok|Dialog.Cancel
  Label{text:qsTr("Save your work before continuing.");color:Prefs.dark?"#eeeeef":"#262628"}
  onAccepted:root.pending==="logout"?UI.logout():System.action(root.pending)
 }
}
