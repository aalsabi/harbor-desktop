import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GlassCard {
    focus: true
    Keys.onEscapePressed: UI.dismiss()
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14
        HarborLabel {
            text: qsTr("Open windows")
            font.pixelSize: 24
            font.bold: true
        }
        HarborLabel {
            text: Windows.available ? qsTr("Choose a window to return to it") : qsTr("Window management protocol unavailable")
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        Flow {
            Layout.fillWidth: true
            spacing: 6
            Repeater {
                model: Windows.desktops
                delegate: HarborButton {
                    required property var modelData
                    text: modelData.name
                    prominent: modelData.active
                    onClicked: Windows.activateDesktop(modelData.id)
                }
            }
            HarborButton {
                text: "+"
                onClicked: Windows.addDesktop()
            }
        }
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: Windows.windows
            spacing: 10
            delegate: RowLayout {
                required property var modelData
                width: ListView.view.width
                HarborButton {
                    text: modelData.title
                    Layout.fillWidth: true
                    prominent: modelData.active
                    onClicked: {
                        Windows.activate(modelData.id);
                        UI.dismiss();
                    }
                }
                HarborButton {
                    text: "−"
                    onClicked: Windows.minimize(modelData.id)
                }
                HarborButton {
                    text: "×"
                    onClicked: Windows.close(modelData.id)
                }
            }
        }
        HarborButton {
            text: qsTr("Close panel")
            onClicked: UI.dismiss()
        }
    }
}
