import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    default property alias contents: layout.data
    Layout.fillWidth: true
    implicitHeight: layout.implicitHeight + 28
    radius: 12
    color: Prefs.dark ? "#29292f" : "#ffffff"
    border.color: Prefs.dark ? "#45454d" : "#dedee4"
    ColumnLayout {
        id: layout
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: 14
        }
        spacing: 10
    }
}
