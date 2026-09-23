import QtQuick
import QtQuick.Controls
Button {
 id: control
 property bool prominent: false
 implicitHeight: 38; font.pixelSize: 13; hoverEnabled: true
 contentItem: Text { text: control.text; color: control.prominent ? "#062a32" : Prefs.dark ? "#eef4ff" : "#17314c"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font: control.font; elide: Text.ElideRight }
 background: Rectangle { radius: 10; color: !control.enabled ? "#22888888" : control.prominent ? "#81ddd0" : control.hovered ? "#338da9cb" : "#198da9cb"; border.color: "#258da9cb" }
 Accessible.name: text
}
