import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    default property alias contents: groupBody.data
    Layout.fillWidth: true
    implicitHeight: groupBody.implicitHeight + 28
    radius: 10
    color: SettingsTheme.card
    border.color: SettingsTheme.line
    ColumnLayout {
        id: groupBody
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12
    }
}
