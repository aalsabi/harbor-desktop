import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    required property Item settings
    Layout.fillWidth: true
    spacing: 18
    SettingsGroup {
        SettingRow {
            label: qsTr("Brightness")
            Slider {
                Layout.preferredWidth: 200
                from: 5
                to: 100
                enabled: !!System.state.brightnessAvailable
                value: System.state.brightnessPercent || 0
                onMoved: {
                    brightnessTimer.requestedValue = value;
                    brightnessTimer.restart();
                }
                Timer {
                    id: brightnessTimer
                    property real requestedValue: 0
                    interval: 180
                    onTriggered: System.action("brightness", Math.round(requestedValue))
                }
            }
        }
    }
    Loader {
        active: root.settings.section === "Displays"
        Layout.fillWidth: true
        Layout.preferredHeight: item ? item.implicitHeight : 0
        source: active ? "DisplaysPage.qml" : ""
    }
}
