import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    required property Item settings
    Layout.fillWidth: true
    spacing: 18
    Loader {
        active: root.settings.section === "Battery"
        Layout.fillWidth: true
        Layout.preferredHeight: item ? item.implicitHeight : 0
        source: active ? "PowerPage.qml" : ""
    }
    SettingsGroup {
        visible: !!System.state.batteryAvailable
        SettingRow {
            label: qsTr("Battery", "battery status")
            hint: System.state.batteryCharging ? qsTr("Charging") : qsTr("On battery / fully charged")
            SettingsLabel {
                text: Math.round(System.state.batteryPercent || 0) + "%"
                Layout.preferredWidth: 70
            }
        }
    }
    SettingsGroup {
        SettingsLabel {
            text: qsTr("Energy mode")
            font.bold: true
        }
        SettingsNote {
            text: qsTr("Current mode: ") + (System.state.power || qsTr("Unavailable"))
        }
        Repeater {
            model: [
                {
                    id: "power-saver",
                    en: QT_TR_NOOP("Low Power")
                },
                {
                    id: "balanced",
                    en: QT_TR_NOOP("Balanced")
                },
                {
                    id: "performance",
                    en: QT_TR_NOOP("Performance")
                }
            ].filter(x => (System.state.powerProfiles || []).includes(x.id))
            delegate: RadioButton {
                required property var modelData
                text: qsTr(modelData.en)
                checked: System.state.power === modelData.id
                enabled: !!System.state.powerAvailable && !System.busy
                onClicked: System.action("power", modelData.id)
            }
        }
    }
    SettingsNote {
        text: qsTr("Available modes depend on your hardware and power service.")
    }
}
