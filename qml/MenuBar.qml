import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 color: Prefs.dark ? "#80222228" : "#80f7f7ff"
 component PanelButton: HarborButton {
  implicitHeight: 28
  Layout.preferredHeight: 28
  leftPadding: 10; rightPadding: 10
  font.pixelSize: 13
  background: Rectangle {
   radius: 6
   color: parent.down ? "#507b9cb5" : parent.hovered ? (Prefs.dark ? "#354b65" : "#d5e3ee") : "transparent"
   border.width: parent.activeFocus ? 2 : 0
   border.color: "#81ddd0"
  }
 }
 Rectangle {anchors.bottom:parent.bottom;width:parent.width;height:1;color:Prefs.dark?"#40516a":"#c5d1df"}
 RowLayout { anchors.fill: parent; anchors.leftMargin: 18; anchors.rightMargin: 18; spacing: 8
  PanelButton { text: Prefs.language==="ar" ? "◈  هاربور" : "◈  Harbor"; font.bold: true; onClicked: UI.open("launcher"); Accessible.name: "Applications" }

  Text { text: Windows.activeTitle;visible:text!=="Harbor"; Layout.maximumWidth: 160; elide: Text.ElideRight; color: Prefs.dark ? "#eef4ff" : "#435b70"; font.pixelSize: 12 }
  Flickable {
   Layout.fillWidth: true; Layout.minimumWidth: 0; Layout.preferredHeight: 28
   contentWidth: menuRow.width; contentHeight: height; clip: true
   flickableDirection: Flickable.HorizontalFlick; boundsBehavior: Flickable.StopAtBounds
   Row {id:menuRow;spacing:2
    Repeater {model:GlobalMenu.roots;delegate:PanelButton{required property var modelData;text:modelData.label.replace(/&(.)/g,"$1");font.weight:Font.DemiBold;onClicked:{GlobalMenu.select(modelData.id);UI.open("menu")}}}
   }
   ScrollBar.horizontal: ScrollBar {height:3;policy:ScrollBar.AsNeeded}
  }
  Repeater{model:Tray.items;delegate:PanelButton{required property var modelData;width:28;height:28;visible:modelData.status!=="Passive";text:"";Image{anchors.centerIn:parent;width:18;height:18;source:"image://icons/"+modelData.icon} onClicked:Tray.activate(modelData.id);onPressAndHold:Tray.activate(modelData.id,true);ToolTip.visible:hovered;ToolTip.text:modelData.title}}
  PanelButton { text: "⌕"; onClicked: UI.open("launcher"); Accessible.name: "Search applications" }
  PanelButton { text: Prefs.language==="ar" ? "النوافذ" : "Windows"; onClicked: UI.open("windows") }
  PanelButton {text:"● "+Notifications.items.length;onClicked:UI.open("notifications");Accessible.name:"Notifications"}
  PanelButton { text: "☷";ToolTip.visible:hovered;ToolTip.text:Prefs.language==="ar"?"مركز التحكم":"Control center"; onClicked: UI.open("control"); Accessible.name: "Control center" }
  Text { id: clock; color: Prefs.dark ? "#eef4ff" : "#163047"; font.pixelSize: 12; text: Qt.formatDateTime(new Date(),"ddd d MMM   hh:mm"); Timer { interval: 1000; running: true; repeat: true; onTriggered: clock.text=Qt.formatDateTime(new Date(),"ddd d MMM   hh:mm") } }
 }
}
