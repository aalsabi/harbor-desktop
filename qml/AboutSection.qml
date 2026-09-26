import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    required property Item settings
    Layout.fillWidth: true
    spacing: 18
    Rectangle {
        width: 80
        height: 80
        radius: 18
        Layout.alignment: Qt.AlignHCenter
        color: "#4888db"
        Text {
            anchors.centerIn: parent
            text: "◈"
            color: "white"
            font.pixelSize: 60
        }
    }
    SettingsLabel {
        text: "Harbor Desktop"
        font.pixelSize: 26
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
    }
    SettingsNote {
        text: qsTr("Version ") + HarborVersion
        horizontalAlignment: Text.AlignHCenter
    }
    SettingsGroup {
        SettingRow {
            label: qsTr("Window system")
            SettingsLabel {
                text: "KWin · Wayland"
                Layout.preferredWidth: 170
            }
        }
        SettingsDivider {}
        SettingRow {
            label: qsTr("Interface")
            SettingsLabel {
                text: "Harbor · Qt 6"
                Layout.preferredWidth: 170
            }
        }
        SettingsDivider {}
        SettingRow {
            label: qsTr("License")
            SettingsLabel {
                text: "GPL-3.0-or-later"
                Layout.preferredWidth: 170
            }
        }
    }
    SettingsGroup {
        SettingRow {
            label: qsTr("Operating system")
            SettingsLabel {
                text: System.state.osName || "Debian Linux"
                Layout.preferredWidth: 220
            }
        }
        SettingsDivider {}
        SettingRow {
            label: qsTr("Architecture")
            SettingsLabel {
                text: System.state.architecture || "—"
                Layout.preferredWidth: 220
            }
        }
    }
    SettingsNote {
        text: qsTr("An independent open-source desktop with original artwork. Plasma Shell is not used.")
    }
}
