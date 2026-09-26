import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    required property Item settings
    Layout.fillWidth: true
    spacing: 18
    Loader {
        active: root.settings.section === "Sound"
        Layout.fillWidth: true
        Layout.preferredHeight: item ? item.implicitHeight : 0
        source: active ? "AudioStreamsPage.qml" : ""
    }
    SettingsGroup {
        SettingsLabel {
            text: qsTr("Output")
            font.bold: true
        }
        SettingRow {
            label: qsTr("Output volume")
            SettingsLabel {
                Layout.preferredWidth: 90
                text: Math.round((parseFloat((System.state.volume || "Volume: 0").split(" ")[1]) || 0) * 100) + "%"
            }
        }
        Slider {
            Layout.fillWidth: true
            from: 0
            to: 1
            value: parseFloat((System.state.volume || "Volume: 0").split(" ")[1]) || 0
            enabled: !!System.state.volumeAvailable
            onMoved: {
                audioTimer.requestedValue = value;
                audioTimer.restart();
            }
            Timer {
                id: audioTimer
                property real requestedValue: 0
                interval: 180
                onTriggered: System.action("volume", requestedValue)
            }
        }
        SettingRow {
            label: qsTr("Mute")
            Switch {
                checked: (System.state.volume || "").includes("MUTED")
                enabled: !!System.state.volumeAvailable && !System.busy
                onToggled: System.action("mute")
            }
        }
    }
    SettingsGroup {
        SettingRow {
            label: qsTr("Output device")
            SettingsSelect {
                objectName: "audio-output"
                Layout.preferredWidth: 260
                model: System.state.audioOutputs || []
                textRole: "name"
                currentIndex: (System.state.audioOutputs || []).findIndex(x => x.id === System.state.defaultOutputId)
                enabled: count > 0 && !System.busy
                onActivated: if (currentIndex >= 0)
                    System.action("audio-output", model[currentIndex].id)
            }
        }
        SettingsDivider {}
        SettingRow {
            label: qsTr("Input device")
            SettingsSelect {
                objectName: "audio-input"
                Layout.preferredWidth: 260
                model: System.state.audioInputs || []
                textRole: "name"
                currentIndex: (System.state.audioInputs || []).findIndex(x => x.id === System.state.defaultInputId)
                enabled: count > 0 && !System.busy
                onActivated: if (currentIndex >= 0)
                    System.action("audio-input", model[currentIndex].id)
            }
        }
        SettingsLabel {
            text: qsTr("Input volume")
            font.bold: true
        }
        Slider {
            Layout.fillWidth: true
            from: 0
            to: 1
            value: Number(System.state.inputVolume) || 0
            enabled: !!System.state.inputVolumeAvailable
            onMoved: {
                inputTimer.requestedValue = value;
                inputTimer.restart();
            }
            Timer {
                id: inputTimer
                property real requestedValue: 0
                interval: 180
                onTriggered: System.action("input-volume", requestedValue)
            }
        }
        SettingRow {
            label: qsTr("Mute microphone")
            Switch {
                checked: !!System.state.inputMuted
                enabled: !!System.state.inputVolumeAvailable && !System.busy
                onToggled: System.action("input-mute")
            }
        }
    }
}
