import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    property string label
    property string hint: ""
    default property alias controls: controlRow.data
    Layout.fillWidth: true
    spacing: 18
    ColumnLayout {
        Layout.fillWidth: true
        spacing: 4
        SettingsLabel {
            text: parent.parent.label
        }
        SettingsNote {
            text: parent.parent.hint
            visible: text.length > 0
        }
    }
    RowLayout {
        id: controlRow
        spacing: 8
    }
}
