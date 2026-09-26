import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GlassCard {
    id: root
    focus: true
    Keys.onEscapePressed: UI.dismiss()
    LayoutMirroring.enabled: Prefs.language === "ar"
    LayoutMirroring.childrenInherit: true
    component Section: Rectangle {
        default property alias contents: body.data
        Layout.fillWidth: true
        implicitHeight: body.implicitHeight + 24
        radius: 14
        color: Prefs.dark ? "#aa33333a" : "#ccffffff"
        border.color: Prefs.dark ? "#44555560" : "#33777788"
        ColumnLayout {
            id: body
            anchors.fill: parent
            anchors.margins: 12
            spacing: 8
        }
    }
    component Tile: HarborButton {
        property string glyph
        property string label
        property string detail
        property bool on: false
        Layout.fillWidth: true
        implicitHeight: 76
        text: ""
        Accessible.name: label
        background: Rectangle {
            radius: 14
            color: parent.on ? Prefs.accent : Prefs.dark ? "#aa33333a" : "#ccffffff"
        }
        contentItem: Column {
            spacing: 4
            Text {
                text: parent.parent.glyph + "  " + parent.parent.label
                color: parent.parent.on ? "white" : Prefs.dark ? "#eeeeef" : "#262628"
                font.bold: true
                font.pixelSize: 14
            }
            Text {
                text: parent.parent.detail
                color: parent.parent.on ? "#e9f4ff" : Prefs.dark ? "#b9b9c3" : "#72727b"
                font.pixelSize: 11
            }
        }
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12
        RowLayout {
            HarborLabel {
                text: qsTr("Control Center")
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
            }
            HarborButton {
                text: "↻"
                Accessible.name: "Refresh controls"
                onClicked: System.refresh()
            }
            HarborButton {
                text: "×"
                Accessible.name: "Close control center"
                onClicked: UI.dismiss()
            }
        }
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth
            clip: true
            ColumnLayout {
                width: parent.width
                spacing: 10
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Tile {
                        objectName: "control-wifi"
                        glyph: "◔"
                        label: "Wi-Fi"
                        detail: !System.state.wifiAvailable ? qsTr("Unavailable") : on ? qsTr("On") : qsTr("Off")
                        on: System.state.wifi === "enabled"
                        enabled: !!System.state.wifiAvailable && !System.busy
                        onClicked: System.action("wifi", !on)
                    }
                    Tile {
                        objectName: "control-bluetooth"
                        glyph: "ᛒ"
                        label: "Bluetooth"
                        detail: !System.state.bluetoothAvailable ? qsTr("Unavailable") : on ? qsTr("On") : qsTr("Off")
                        on: (System.state.bluetooth || "").includes("Powered: yes")
                        enabled: !!System.state.bluetoothAvailable && !System.state.bluetoothBusy
                        onClicked: System.action("bluetooth", !on)
                    }
                }
                RowLayout {
                    HarborButton {
                        text: qsTr("Networks…")
                        Layout.fillWidth: true
                        onClicked: UI.open("settings:Wi-Fi")
                    }
                    HarborButton {
                        text: qsTr("Devices…")
                        Layout.fillWidth: true
                        onClicked: UI.open("settings:Bluetooth")
                    }
                }
                Section {
                    visible: typeof NotificationPrefs !== "undefined"
                    RowLayout {
                        HarborLabel {
                            Layout.fillWidth: true
                            text: qsTr("Do Not Disturb")
                            font.bold: true
                        }
                        Switch {
                            checked: typeof NotificationPrefs !== "undefined" && NotificationPrefs.doNotDisturb
                            onClicked: if (typeof NotificationPrefs !== "undefined")
                                NotificationPrefs.doNotDisturb = checked
                        }
                    }
                    HarborLabel {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        text: typeof NotificationPrefs !== "undefined" ? (NotificationPrefs.activeProfile.name || qsTr("No active Focus profile")) : ""
                    }
                    HarborButton {
                        text: qsTr("Focus schedules…")
                        onClicked: UI.open("settings:Notifications & Focus")
                    }
                }
                Section {
                    RowLayout {
                        HarborLabel {
                            text: qsTr("Display")
                            font.bold: true
                            Layout.fillWidth: true
                        }
                        HarborLabel {
                            text: System.state.brightnessAvailable ? Math.round(System.state.brightnessPercent || 0) + "%" : qsTr("Unavailable")
                        }
                    }
                    Slider {
                        objectName: "control-brightness"
                        Layout.fillWidth: true
                        from: 5
                        to: 100
                        value: System.state.brightnessPercent || 0
                        enabled: !!System.state.brightnessAvailable
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
                    RowLayout {
                        HarborButton {
                            text: Prefs.dark ? qsTr("Light") : qsTr("Dark")
                            onClicked: Prefs.dark = !Prefs.dark
                        }
                        HarborButton {
                            text: qsTr("Displays…")
                            onClicked: UI.open("settings:Displays")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                    }
                }
                Section {
                    RowLayout {
                        HarborLabel {
                            text: qsTr("Sound")
                            font.bold: true
                            Layout.fillWidth: true
                        }
                        HarborLabel {
                            text: System.state.volumeAvailable ? Math.round((System.state.outputVolume || 0) * 100) + "%" : qsTr("Unavailable")
                        }
                    }
                    Slider {
                        objectName: "control-volume"
                        Layout.fillWidth: true
                        from: 0
                        to: 1
                        value: System.state.outputVolume || 0
                        enabled: !!System.state.volumeAvailable
                        onMoved: {
                            volumeTimer.requestedValue = value;
                            volumeTimer.restart();
                        }
                        Timer {
                            id: volumeTimer
                            property real requestedValue: 0
                            interval: 180
                            onTriggered: System.action("volume", requestedValue)
                        }
                    }
                    RowLayout {
                        HarborButton {
                            text: System.state.outputMuted ? qsTr("Unmute") : qsTr("Mute")
                            enabled: !!System.state.volumeAvailable && !System.busy
                            onClicked: System.action("mute")
                        }
                        HarborButton {
                            text: qsTr("Output & Input…")
                            onClicked: UI.open("settings:Sound")
                        }
                    }
                }
                Section {
                    RowLayout {
                        HarborLabel {
                            text: qsTr("Keyboard")
                            font.bold: true
                            Layout.fillWidth: true
                        }
                        HarborButton {
                            text: qsTr("Switch")
                            enabled: !!System.state.harborSession
                            onClicked: Keyboard.switchNext()
                        }
                        HarborButton {
                            text: "…"
                            Accessible.name: "Keyboard settings"
                            onClicked: UI.open("settings:Keyboard")
                        }
                    }
                    HarborLabel {
                        text: Keyboard.message
                        visible: text.length > 0
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.pixelSize: 11
                    }
                }
                Section {
                    RowLayout {
                        HarborLabel {
                            text: qsTr("Power")
                            font.bold: true
                            Layout.fillWidth: true
                        }
                        HarborLabel {
                            text: System.state.batteryAvailable ? Math.round(System.state.batteryPercent) + "%" + (System.state.batteryCharging ? " ⚡" : "") : ""
                        }
                    }
                    ComboBox {
                        Layout.fillWidth: true
                        model: System.state.powerProfiles || []
                        currentIndex: (System.state.powerProfiles || []).indexOf(System.state.power || "")
                        enabled: !!System.state.powerAvailable && !System.busy
                        onActivated: if (currentIndex >= 0)
                            System.action("power", currentText)
                    }
                    HarborButton {
                        text: qsTr("Battery settings…")
                        onClicked: UI.open("settings:Battery")
                    }
                }
                HarborButton {
                    text: qsTr("Lock Screen")
                    enabled: !!System.state.lockAvailable
                    Layout.fillWidth: true
                    onClicked: System.action("lock")
                }
                SessionActions {
                    Layout.fillWidth: true
                }
                HarborLabel {
                    text: System.message
                    visible: text.length > 0
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    color: Prefs.dark ? "#edbd80" : "#86571e"
                    font.pixelSize: 12
                }
            }
        }
        HarborButton {
            text: qsTr("System Settings…")
            Layout.fillWidth: true
            prominent: true
            onClicked: UI.open("settings")
        }
    }
}
