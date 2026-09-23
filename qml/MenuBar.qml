import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 color: Prefs.dark ? "#d9101b2d" : "#d9f7f9fc"
 RowLayout { anchors.fill: parent; anchors.leftMargin: 18; anchors.rightMargin: 18; spacing: 16
  HarborButton { text: "◈"; font.pixelSize: 22; onClicked: UI.open("launcher"); Accessible.name: "Applications" }
  Text { text: "Harbor"; color: Prefs.dark ? "white" : "#163047"; font.bold: true; font.pixelSize: 13 }
  Text { text: Windows.activeTitle; Layout.maximumWidth: 350; elide: Text.ElideRight; color: Prefs.dark ? "#bdcce0" : "#435b70"; font.pixelSize: 12 }
  Repeater {model:GlobalMenu.roots;delegate:HarborButton{required property var modelData;text:modelData.label;onClicked:{GlobalMenu.select(modelData.id);UI.open("menu")}}}
  Item { Layout.fillWidth: true }
  Repeater{model:Tray.items;delegate:HarborButton{required property var modelData;width:28;height:28;visible:modelData.status!=="Passive";text:"";Image{anchors.centerIn:parent;width:18;height:18;source:"image://icons/"+modelData.icon} onClicked:Tray.activate(modelData.id);onPressAndHold:Tray.activate(modelData.id,true);ToolTip.visible:hovered;ToolTip.text:modelData.title}}
  HarborButton { text: "⌕"; onClicked: UI.open("launcher"); Accessible.name: "Search applications" }
  HarborButton { text: "Windows"; onClicked: UI.open("windows") }
  HarborButton {text:"● "+Notifications.items.length;onClicked:UI.open("notifications");Accessible.name:"Notifications"}
  HarborButton { text: "☷"; onClicked: UI.open("control"); Accessible.name: "Control center" }
  Text { id: clock; color: Prefs.dark ? "#eef4ff" : "#163047"; font.pixelSize: 12; text: Qt.formatDateTime(new Date(),"ddd d MMM   hh:mm"); Timer { interval: 1000; running: true; repeat: true; onTriggered: clock.text=Qt.formatDateTime(new Date(),"ddd d MMM   hh:mm") } }
 }
}
