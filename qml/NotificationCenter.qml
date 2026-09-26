import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GlassCard {
    focus: true
    Keys.onEscapePressed: UI.dismiss()
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        HarborLabel {
            text: qsTr("Notifications")
            font.pixelSize: 24
        }
        HarborLabel {
            text: Notifications.available ? qsTr("Recent notifications") : qsTr("Notification service owned by another desktop")
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 12
            model: Notifications.items
            delegate: ColumnLayout {
                required property var modelData
                width: ListView.view.width
                spacing: 8
                RowLayout {
                    HarborLabel {
                        text: modelData.summary
                        textFormat: Text.PlainText
                        font.bold: true
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                    }
                    HarborButton {
                        text: "×"
                        onClicked: Notifications.CloseNotification(modelData.id)
                    }
                }
                HarborLabel {
                    text: modelData.body
                    textFormat: Text.PlainText
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                Repeater {
                    model: Math.floor(modelData.actions.length / 2)
                    delegate: HarborButton {
                        required property int index
                        text: modelData.actions[index * 2 + 1]
                        onClicked: Notifications.invoke(modelData.id, modelData.actions[index * 2])
                    }
                }
            }
        }
        HarborButton {
            text: qsTr("Close panel")
            onClicked: UI.dismiss()
        }
    }
}
