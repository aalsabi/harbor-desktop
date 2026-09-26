import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    objectName: "accessibilityPage"
    spacing: 14
    Component.onCompleted: AccessibilitySettings.refresh()
    HarborLabel {
        text: qsTr("Vision and screen reading")
        font.bold: true
        font.pixelSize: 17
    }
    Repeater {
        model: [
            {
                id: "zoom",
                en: QT_TR_NOOP("Screen magnification")
            },
            {
                id: "invert",
                en: QT_TR_NOOP("Invert colors")
            },
            {
                id: "trackmouse",
                en: QT_TR_NOOP("Enable pointer locator shortcut")
            },
            {
                id: "mouseclick",
                en: QT_TR_NOOP("Enable mouse-click effect shortcut")
            }
        ]
        delegate: RowLayout {
            required property var modelData
            Layout.fillWidth: true
            HarborLabel {
                Layout.fillWidth: true
                text: qsTr(modelData.en)
            }
            Switch {
                checked: (AccessibilitySettings.state[modelData.id === "invert" ? "active" : "loaded"] || []).includes(modelData.id)
                enabled: !AccessibilitySettings.busy && (AccessibilitySettings.state.available || []).includes(modelData.id)
                Accessible.name: qsTr(modelData.en)
                onClicked: {
                    AccessibilitySettings.setEffect(modelData.id, checked);
                    checked = Qt.binding(function () {
                        return (AccessibilitySettings.state[modelData.id === "invert" ? "active" : "loaded"] || []).includes(modelData.id);
                    });
                }
            }
        }
    }
    RowLayout {
        HarborButton {
            text: qsTr("Zoom in")
            enabled: !AccessibilitySettings.busy && (AccessibilitySettings.state.loaded || []).includes("zoom")
            onClicked: AccessibilitySettings.zoom("in")
        }
        HarborButton {
            text: qsTr("Zoom out")
            enabled: !AccessibilitySettings.busy && (AccessibilitySettings.state.loaded || []).includes("zoom")
            onClicked: AccessibilitySettings.zoom("out")
        }
        HarborButton {
            text: qsTr("Actual size")
            enabled: !AccessibilitySettings.busy && (AccessibilitySettings.state.loaded || []).includes("zoom")
            onClicked: AccessibilitySettings.zoom("reset")
        }
    }
    HarborLabel {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Visual aids depend on compositor support. Pointer location and click effects use their configured KWin shortcuts.")
    }
    RowLayout {
        Layout.fillWidth: true
        HarborLabel {
            Layout.fillWidth: true
            text: qsTr("Orca screen reader")
        }
        Switch {
            checked: !!AccessibilitySettings.state.screenReaderEnabled
            enabled: !!AccessibilitySettings.state.orcaAvailable
            Accessible.name: qsTr("Screen reader")
            onClicked: AccessibilitySettings.setScreenReader(checked)
        }
    }
    HarborLabel {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: !AccessibilitySettings.state.orcaAvailable ? qsTr("Install Orca to enable screen reading.") : (AccessibilitySettings.state.readerStatus || "")
    }
    HarborButton {
        text: qsTr("Refresh")
        enabled: !AccessibilitySettings.busy
        onClicked: AccessibilitySettings.refresh()
    }
    HarborLabel {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: AccessibilitySettings.error
    }
}
