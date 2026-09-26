import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    required property Item settings
    Layout.fillWidth: true
    spacing: 18
    Loader {
        active: root.settings.section === "Accessibility"
        Layout.fillWidth: true
        Layout.preferredHeight: item ? item.implicitHeight : 0
        source: active ? "AccessibilityPage.qml" : ""
    }
    SettingsGroup {
        SettingRow {
            label: qsTr("Reduce motion")
            hint: qsTr("Reduce animation in Harbor")
            Switch {
                checked: Prefs.reduceMotion
                onToggled: Prefs.reduceMotion = checked
            }
        }
    }
    SettingsGroup {
        SettingRow {
            label: qsTr("Reduce transparency")
            Switch {
                checked: Prefs.opacity === 1
                onToggled: {
                    if (checked) {
                        root.settings.previousOpacity = Prefs.opacity;
                        Prefs.opacity = 1;
                    } else
                        Prefs.opacity = root.settings.previousOpacity;
                }
            }
        }
    }
    SettingsNote {
        text: qsTr("These controls apply to Harbor. Screen-reader, magnifier and assistive-input configuration is not included yet.")
    }
}
