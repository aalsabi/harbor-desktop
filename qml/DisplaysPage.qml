pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    spacing: 16
    property color ink: Prefs.dark ? "#eeeef0" : "#252527"
    property color muted: Prefs.dark ? "#a6a6ad" : "#76767c"
    Component.onCompleted: DisplaySettings.refresh()
    component Label: Text {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: root.ink
        font.pixelSize: 13
    }
    component Group: Rectangle {
        default property alias contents: body.data
        Layout.fillWidth: true
        implicitHeight: body.implicitHeight + 28
        radius: 10
        color: Prefs.dark ? "#303034" : "white"
        border.color: Prefs.dark ? "#454549" : "#e2e2e7"
        ColumnLayout {
            id: body
            anchors.fill: parent
            anchors.margins: 14
            spacing: 12
        }
    }
    Label {
        text: qsTr("Arrange connected displays using logical pixel positions. Display changes revert after 15 seconds unless you keep them.")
        color: root.muted
    }
    Group {
        visible: DisplaySettings.pending
        Label {
            text: qsTr("Keep this display layout?")
            font.bold: true
        }
        RowLayout {
            HarborButton {
                text: qsTr("Keep changes")
                onClicked: DisplaySettings.confirm()
            }
            HarborButton {
                text: qsTr("Revert now")
                onClicked: DisplaySettings.revert()
            }
        }
    }
    Label {
        visible: DisplaySettings.error.length > 0
        text: DisplaySettings.error
    }
    Repeater {
        model: DisplaySettings.outputs
        delegate: Group {
            id: display
            required property var modelData
            Label {
                text: display.modelData.name + (display.modelData.priority === 1 ? qsTr(" · Primary") : "")
                font.bold: true
                font.pixelSize: 15
            }
            ComboBox {
                id: mode
                Layout.fillWidth: true
                model: display.modelData.modes
                textRole: "name"
                currentIndex: display.modelData.modes.findIndex(m => m.id === display.modelData.currentModeId)
                enabled: !DisplaySettings.busy
                Accessible.name: qsTr("Resolution and refresh rate")
                displayText: currentIndex >= 0 ? model[currentIndex].size.width + " × " + model[currentIndex].size.height + " · " + Math.round(model[currentIndex].refreshRate * 100) / 100 + " Hz" : ""
                delegate: ItemDelegate {
                    required property var modelData
                    required property int index
                    width: mode.width
                    text: modelData.size.width + " × " + modelData.size.height + " · " + Math.round(modelData.refreshRate * 100) / 100 + " Hz"
                    highlighted: mode.highlightedIndex === index
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Label {
                    text: qsTr("Scale")
                }
                SpinBox {
                    id: scale
                    from: 50
                    to: 400
                    stepSize: 25
                    value: Math.round(display.modelData.scale * 100)
                    editable: true
                    enabled: !DisplaySettings.busy
                    Accessible.name: qsTr("Scale percent")
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Label {
                    text: qsTr("Horizontal position")
                }
                SpinBox {
                    id: posX
                    from: 0
                    to: 32768
                    value: display.modelData.pos.x
                    editable: true
                    enabled: !DisplaySettings.busy
                    Accessible.name: qsTr("Horizontal position")
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Label {
                    text: qsTr("Vertical position")
                }
                SpinBox {
                    id: posY
                    from: 0
                    to: 32768
                    value: display.modelData.pos.y
                    editable: true
                    enabled: !DisplaySettings.busy
                    Accessible.name: qsTr("Vertical position")
                }
            }
            CheckBox {
                id: primary
                text: qsTr("Use as primary display")
                checked: display.modelData.priority === 1
                enabled: !DisplaySettings.busy && display.modelData.priority !== 1
            }
            HarborButton {
                text: qsTr("Apply display changes")
                enabled: !DisplaySettings.busy && mode.currentIndex >= 0
                onClicked: DisplaySettings.apply(display.modelData.id, posX.value, posY.value, scale.value / 100, mode.model[mode.currentIndex].id, primary.checked)
            }
        }
    }
    HarborButton {
        text: qsTr("Refresh displays")
        enabled: !DisplaySettings.busy
        onClicked: DisplaySettings.refresh()
    }
    Group {
        Label {
            text: qsTr("External monitor brightness")
            font.bold: true
        }
        Label {
            text: qsTr("Available for monitors that support DDC/CI. Detection may take a few seconds.")
            color: root.muted
        }
        Label {
            visible: DisplaySettings.brightnessError.length > 0
            text: DisplaySettings.brightnessError
            color: root.muted
        }
        Repeater {
            model: DisplaySettings.monitors
            delegate: ColumnLayout {
                id: monitor
                required property var modelData
                Layout.fillWidth: true
                Label {
                    text: monitor.modelData.name
                }
                RowLayout {
                    Layout.fillWidth: true
                    Slider {
                        id: brightness
                        Layout.fillWidth: true
                        from: 0
                        to: 100
                        stepSize: 1
                        value: monitor.modelData.percent
                        enabled: !DisplaySettings.busy
                        Accessible.name: qsTr("Monitor brightness")
                    }
                    HarborButton {
                        text: qsTr("Apply")
                        enabled: !DisplaySettings.busy
                        onClicked: DisplaySettings.setBrightness(monitor.modelData.bus, Math.round(brightness.value))
                    }
                }
            }
        }
        HarborButton {
            text: qsTr("Detect brightness controls")
            enabled: !DisplaySettings.busy
            onClicked: DisplaySettings.detectBrightness()
        }
    }
}
