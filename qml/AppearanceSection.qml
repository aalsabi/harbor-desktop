import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    required property Item settings
    component ThemeChoice: Button {
        property bool night: false
        implicitWidth: 154
        implicitHeight: 105
        onClicked: Prefs.dark = night
        Accessible.name: night ? "Dark appearance" : "Light appearance"
        background: Rectangle {
            radius: 9
            color: parent.night ? "#22222b" : "#e0eaff"
            border.width: Prefs.dark === parent.night ? 3 : 1
            border.color: Prefs.dark === parent.night ? Prefs.accent : SettingsTheme.line
            Rectangle {
                anchors.centerIn: parent
                width: 112
                height: 72
                radius: 6
                color: parent.parent.night ? "#36363d" : "#ffffff"
                Rectangle {
                    width: 30
                    height: parent.height
                    radius: 6
                    color: parent.parent.parent.night ? "#4a4a52" : "#e8e8ef"
                }
                Column {
                    anchors.centerIn: parent
                    anchors.horizontalCenterOffset: 12
                    spacing: 7
                    Repeater {
                        model: 3
                        Rectangle {
                            width: 54
                            height: 5
                            radius: 2
                            color: night ? "#60606d" : "#d9d9e2"
                        }
                    }
                }
            }
        }
        contentItem: Item {}
    }
    Layout.fillWidth: true
    spacing: 18
    SettingsGroup {
        SettingsLabel {
            text: qsTr("Appearance")
            font.bold: true
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 22
            ColumnLayout {
                ThemeChoice {
                    night: false
                    objectName: "appearance-light"
                }
                SettingsLabel {
                    text: qsTr("Light")
                    horizontalAlignment: Text.AlignHCenter
                }
            }
            ColumnLayout {
                ThemeChoice {
                    night: true
                    objectName: "appearance-dark"
                }
                SettingsLabel {
                    text: qsTr("Dark")
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
    SettingsGroup {
        SettingRow {
            label: qsTr("Accent colour")
            Row {
                spacing: 8
                Repeater {
                    model: ["#1684f8", "#168044", "#7955c9", "#b65b00", "#c63f75"]
                    delegate: Button {
                        required property string modelData
                        width: 28
                        height: 28
                        Accessible.name: modelData
                        contentItem: Item {}
                        background: Rectangle {
                            radius: 14
                            color: parent.modelData
                            border.width: Prefs.accent === parent.modelData ? 3 : 0
                            border.color: SettingsTheme.ink
                        }
                        onClicked: Prefs.accent = modelData
                    }
                }
            }
        }
    }
    SettingsNote {
        text: qsTr("Appearance applies to Harbor windows and desktop panels. Other apps use their own themes.")
    }
}
