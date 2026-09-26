pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    objectName: "bluetoothSettingsPage"
    readonly property bool arabic: Prefs.language === "ar"
    readonly property var state: System.state
    readonly property var prompt: state.bluetoothPrompt || ({})
    property string forgetPath: ""
    LayoutMirroring.enabled: arabic
    LayoutMirroring.childrenInherit: true
    spacing: 16
    component Note: HarborLabel {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: Prefs.dark ? "#aaaab2" : "#696971"
    }
    component Card: Rectangle {
        default property alias contents: layout.data
        Layout.fillWidth: true
        implicitHeight: layout.implicitHeight + 28
        radius: 12
        color: Prefs.dark ? "#29292f" : "#ffffff"
        border.color: Prefs.dark ? "#45454d" : "#dedee4"
        ColumnLayout {
            id: layout
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: 14
            }
            spacing: 10
        }
    }
    Card {
        RowLayout {
            Layout.fillWidth: true
            HarborLabel {
                Layout.fillWidth: true
                text: "Bluetooth"
                font.bold: true
            }
            Switch {
                Accessible.name: "Bluetooth"
                checked: !!root.state.bluetoothPowered
                enabled: !!root.state.bluetoothAvailable && !root.state.bluetoothBusy
                onClicked: {
                    System.action("bluetooth", checked);
                    checked = Qt.binding(function () {
                        return !!root.state.bluetoothPowered;
                    });
                }
            }
        }
        HarborButton {
            text: qsTr("Search for devices")
            enabled: !!root.state.bluetoothPowered && !root.state.bluetoothBusy
            onClicked: System.action("bluetooth-scan")
        }
        Note {
            visible: !root.state.bluetoothAvailable
            text: qsTr("Bluetooth service or adapter unavailable.")
        }
    }
    Card {
        visible: !!root.prompt.kind
        HarborLabel {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            font.bold: true
            text: qsTr("Pair with ") + (root.prompt.name || "")
        }
        Note {
            text: root.prompt.kind === "confirm" ? qsTr("Confirm that this code is shown on the other device.") : root.prompt.kind === "display" ? qsTr("Enter this code on the other device.") : root.prompt.kind === "authorize" ? qsTr("Allow this device to pair or access the requested service?") : qsTr("Enter the PIN or passkey shown by the device.")
        }
        HarborLabel {
            visible: !!root.prompt.code
            text: root.prompt.code || ""
            font.pixelSize: 24
            font.bold: true
            Layout.fillWidth: true
            wrapMode: Text.WrapAnywhere
        }
        HarborField {
            id: pin
            Layout.fillWidth: true
            visible: root.prompt.kind === "pin" || root.prompt.kind === "passkey"
            echoMode: TextInput.Password
            placeholderText: qsTr("PIN / passkey")
            Accessible.name: placeholderText
            onVisibleChanged: text = ""
        }
        RowLayout {
            HarborButton {
                visible: root.prompt.kind !== "display"
                text: qsTr("Accept")
                onClicked: {
                    System.action("bluetooth-respond", {
                        accept: true,
                        value: pin.text
                    });
                    pin.text = "";
                }
            }
            HarborButton {
                text: qsTr("Cancel")
                onClicked: {
                    System.action("bluetooth-cancel-pair");
                    pin.text = "";
                }
            }
        }
    }
    Repeater {
        model: root.state.bluetoothDevices || []
        delegate: Card {
            id: device
            required property var modelData
            HarborLabel {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: device.modelData.name
                font.bold: true
            }
            Note {
                text: device.modelData.connected ? qsTr("Connected") : device.modelData.paired ? qsTr("Paired") : qsTr("Not paired")
            }
            Flow {
                Layout.fillWidth: true
                spacing: 8
                HarborButton {
                    text: !device.modelData.paired ? qsTr("Pair") : device.modelData.connected ? qsTr("Disconnect") : qsTr("Connect")
                    enabled: !root.state.bluetoothBusy && !!root.state.bluetoothPowered
                    onClicked: System.action(!device.modelData.paired ? "bluetooth-pair" : device.modelData.connected ? "bluetooth-disconnect" : "bluetooth-connect", !device.modelData.paired ? device.modelData.path : device.modelData.address)
                }
                HarborButton {
                    visible: device.modelData.paired
                    text: qsTr("Forget device")
                    enabled: !root.state.bluetoothBusy
                    onClicked: root.forgetPath = device.modelData.path
                }
            }
            ColumnLayout {
                visible: root.forgetPath === device.modelData.path
                Layout.fillWidth: true
                Note {
                    text: qsTr("Remove this pairing? You will need to pair again to reconnect.")
                }
                RowLayout {
                    HarborButton {
                        text: qsTr("Forget")
                        onClicked: {
                            System.action("bluetooth-unpair", root.forgetPath);
                            root.forgetPath = "";
                        }
                    }
                    HarborButton {
                        text: qsTr("Cancel")
                        onClicked: root.forgetPath = ""
                    }
                }
            }
        }
    }
    Note {
        visible: !!root.state.bluetoothError
        text: root.state.bluetoothError || ""
    }
    Note {
        visible: !!root.state.bluetoothBusy
        text: qsTr("Waiting for the Bluetooth device…")
    }
    HarborButton {
        visible: !!root.state.bluetoothBusy
        text: qsTr("Cancel pairing")
        onClicked: System.action("bluetooth-cancel-pair")
    }
    Component.onDestruction: {
        System.action("bluetooth-stop-scan");
        System.action("bluetooth-cancel-pair");
    }
}
