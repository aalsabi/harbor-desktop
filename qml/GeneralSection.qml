import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    required property Item settings
    component LinkRow: Button {
        property string destination
        Layout.fillWidth: true
        implicitHeight: 39
        text: destination
        onClicked: root.settings.section = destination
        background: Rectangle {
            radius: 6
            color: parent.hovered ? (Prefs.dark ? "#45454b" : "#f0f0f5") : "transparent"
        }
        contentItem: RowLayout {
            SettingsLabel {
                text: parent.parent.text
            }
            Text {
                text: Prefs.language === "ar" ? "‹" : "›"
                font.pixelSize: 23
                color: SettingsTheme.muted
            }
        }
    }
    Layout.fillWidth: true
    spacing: 18
    Rectangle {
        Layout.alignment: Qt.AlignHCenter
        width: 64
        height: 64
        radius: 15
        color: "#888a93"
        Text {
            anchors.centerIn: parent
            text: "⚙"
            font.pixelSize: 49
            color: "white"
        }
    }
    SettingsLabel {
        text: qsTr("General")
        font.pixelSize: 24
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
    }
    SettingsNote {
        text: qsTr("Manage your desktop, language and system information.")
        horizontalAlignment: Text.AlignHCenter
    }
    SettingsGroup {
        LinkRow {
            destination: "About"
            text: qsTr("About")
        }
        SettingsDivider {}
        LinkRow {
            destination: "Software Update"
            text: qsTr("Software Update")
        }
    }
    SettingsGroup {
        LinkRow {
            destination: "Date & Time"
            text: qsTr("Date & Time")
        }
        SettingsDivider {}
        LinkRow {
            destination: "Storage"
            text: qsTr("Storage")
        }
        SettingsDivider {}
        LinkRow {
            destination: "Default Applications"
            text: qsTr("Default Applications")
        }
    }
    SettingsGroup {
        LinkRow {
            destination: "Language & Region"
            text: qsTr("Language & Region")
        }
        SettingsDivider {}
        LinkRow {
            destination: "Keyboard"
            text: qsTr("Keyboard input sources")
        }
    }
    SettingsGroup {
        SettingsLabel {
            text: qsTr("Session")
            font.bold: true
        }
        SessionActions {}
        SettingsNote {
            text: qsTr("Save your work before signing out or powering off.")
        }
    }
}
