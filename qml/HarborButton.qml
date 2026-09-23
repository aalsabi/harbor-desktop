import QtQuick
import QtQuick.Controls
Button {
 id: control
 property bool prominent: false
 implicitHeight: 30; font.pixelSize: 13; hoverEnabled: true
 contentItem: Text { text: control.text; color: control.prominent ? "#ffffff" : Prefs.dark ? "#eeeeef" : "#29292d"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font: control.font; elide: Text.ElideRight }
 background: Rectangle { radius: 7; color: !control.enabled ? "#22888888" : control.prominent ? "#1684f8" : control.hovered ? (Prefs.dark?"#515158":"#e3e3e9") : (Prefs.dark?"#3b3b40":"#f8f8fa"); border.color: (Prefs.dark?"#55555b":"#d9d9df") }
 Accessible.name: text
}
