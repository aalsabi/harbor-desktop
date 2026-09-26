import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: themedDialog
    padding: 18
    spacing: 12
    background: Rectangle {
        radius: 12
        color: Prefs.dark ? "#29292e" : "#f7f7f9"
        border.color: RegionTheme.line
    }
    header: HarborLabel {
        text: themedDialog.title
        font.bold: true
        font.pixelSize: 14
        padding: 18
        bottomPadding: 6
        wrapMode: Text.WordWrap
        horizontalAlignment: RegionTheme.arabic ? Text.AlignRight : Text.AlignLeft
    }
}
