import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ComboBox {
    id: select
    implicitHeight: 30
    implicitWidth: 170
    font.pixelSize: 13
    leftPadding: 10
    rightPadding: 27
    contentItem: Text {
        text: select.displayText
        color: SettingsTheme.ink
        font: select.font
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: 6
        color: Prefs.dark ? "#45454b" : "#fafafa"
        border.color: select.activeFocus ? Prefs.accent : SettingsTheme.line
        border.width: select.activeFocus ? 2 : 1
    }
    indicator: Text {
        x: select.width - width - 9
        y: (select.height - height) / 2
        text: "⌄"
        color: SettingsTheme.muted
        font.pixelSize: 16
    }
}
