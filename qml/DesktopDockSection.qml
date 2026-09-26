import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    required property Item settings
    Layout.fillWidth: true
    spacing: 18
    SettingsGroup {
        Image {
            Layout.fillWidth: true
            Layout.preferredHeight: 170
            source: "qrc:/assets/wallpapers/" + Prefs.wallpaper + ".svg"
            fillMode: Image.PreserveAspectCrop
            clip: true
        }
        SettingsNote {
            text: qsTr("Harbor • Original artwork")
        }
    }
    SettingsGroup {
        SettingRow {
            label: qsTr("Wallpaper")
            SettingsSelect {
                model: ["Harbor", "Sunset", "Forest"]
                property var keys: ["harbor", "sunset", "forest"]
                currentIndex: keys.indexOf(Prefs.wallpaper)
                onActivated: Prefs.wallpaper = keys[currentIndex]
            }
        }
        SettingsDivider {}
        SettingRow {
            label: qsTr("Dock icon size")
            Slider {
                from: 32
                to: 56
                stepSize: 2
                Layout.preferredWidth: 190
                value: Prefs.dockIconSize
                onMoved: Prefs.dockIconSize = Math.round(value)
            }
        }
    }
    SettingsGroup {
        SettingRow {
            label: qsTr("Panel opacity")
            SettingsLabel {
                Layout.preferredWidth: 60
                text: Math.round(Prefs.opacity * 100) + "%"
            }
        }
        Slider {
            Layout.fillWidth: true
            from: .45
            to: 1
            value: Prefs.opacity
            onMoved: Prefs.opacity = value
        }
    }
    SettingsGroup {
        SettingsLabel {
            text: qsTr("Pinned applications")
            font.bold: true
        }
        SettingsNote {
            text: qsTr("Right-click an app in the launcher to pin or unpin it. Press and hold a pinned Dock icon to remove it.")
        }
    }
}
