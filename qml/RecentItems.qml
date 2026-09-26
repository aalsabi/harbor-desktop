import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GlassCard {
    focus: true
    Keys.onEscapePressed: UI.dismiss()
    Component.onCompleted: System.refreshRecent()
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        HarborLabel {
            text: qsTr("Recent Items")
            font.bold: true
            font.pixelSize: 20
        }
        HarborLabel {
            text: qsTr("No recent files")
            visible: !(System.state.recentItems || []).length
        }
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: System.state.recentItems || []
            spacing: 5
            delegate: HarborButton {
                required property var modelData
                width: ListView.view.width
                text: modelData.title
                onClicked: System.openRecent(modelData.url)
            }
        }
        HarborButton {
            text: qsTr("Back")
            onClicked: UI.open("harbor")
        }
    }
}
