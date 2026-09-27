pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    objectName: "networkSettingsPage"
    spacing: 16
    readonly property bool arabic: Prefs.language === "ar"
    property var secretValues: ({})
    property string selectedAp: ""
    property string editingPath: ""
    property bool editing: false
    property var draft: ({})
    function stage(key, value) {
        var next = Object.assign({}, draft);
        next[key] = value;
        draft = next;
    }
    function enterprise(ssid) {
        edit("");
        stage("enterprise", true);
        stage("eap", "peap");
        stage("phase2Auth", "mschapv2");
        stage("type", "802-11-wireless");
        stage("ssid", ssid);
        stage("name", ssid);
    }
    function edit(path) {
        editingPath = path;
        draft = NetworkSettings.profileDraft(path);
        editing = true;
    }
    LayoutMirroring.enabled: arabic
    LayoutMirroring.childrenInherit: true
    Connections {
        target: NetworkSettings
        function onProfileSaved(path) {
            root.editingPath = path;
            root.editing = false;
            root.draft = {};
        }
        function onChanged() {
            if (!NetworkSettings.secretPrompt.name)
                root.secretValues = {};
        }
    }
    Component.onCompleted: NetworkSettings.setActive(true)
    Component.onDestruction: NetworkSettings.setActive(false)
    NetworkCard {
        RowLayout {
            Layout.fillWidth: true
            HarborLabel {
                Layout.fillWidth: true
                text: qsTr("Wi-Fi")
                font.bold: true
            }
            Switch {
                Accessible.name: qsTr("Wi-Fi")
                checked: NetworkSettings.wirelessEnabled
                enabled: NetworkSettings.available && !NetworkSettings.busy
                onClicked: {
                    NetworkSettings.setWirelessEnabled(checked);
                    checked = Qt.binding(function () {
                        return NetworkSettings.wirelessEnabled;
                    });
                }
            }
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            HarborButton {
                text: qsTr("Scan networks")
                enabled: NetworkSettings.available && NetworkSettings.wirelessEnabled && !NetworkSettings.busy
                onClicked: NetworkSettings.scan()
            }
            HarborButton {
                text: qsTr("Refresh")
                enabled: !NetworkSettings.busy
                onClicked: NetworkSettings.refresh()
            }
        }
        NetworkNote {
            visible: !NetworkSettings.available
            text: qsTr("NetworkManager is unavailable.")
        }
    }
    NetworkCard {
        visible: !!NetworkSettings.secretPrompt.name
        HarborLabel {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Authentication: ") + (NetworkSettings.secretPrompt.name || "")
            font.bold: true
        }
        NetworkNote {
            text: qsTr("Enter the credentials requested for this connection attempt.")
        }
        Repeater {
            model: NetworkSettings.secretPrompt.fields || []
            delegate: HarborField {
                required property var modelData
                Layout.fillWidth: true
                placeholderText: modelData.label
                Accessible.name: placeholderText
                echoMode: modelData.secret ? TextInput.Password : TextInput.Normal
                onTextEdited: {
                    var next = Object.assign({}, root.secretValues);
                    next[modelData.key] = text;
                    root.secretValues = next;
                }
            }
        }
        RowLayout {
            HarborButton {
                text: qsTr("Continue")
                onClicked: {
                    NetworkSettings.respondSecrets(true, root.secretValues);
                    if (!NetworkSettings.secretPrompt.name)
                        root.secretValues = {};
                }
            }
            HarborButton {
                text: qsTr("Cancel")
                onClicked: {
                    NetworkSettings.cancelSecrets();
                    root.secretValues = {};
                }
            }
        }
    }
    Repeater {
        model: NetworkSettings.devices
        delegate: NetworkCard {
            id: device
            required property var modelData
            HarborLabel {
                text: device.modelData.interface + " · " + (device.modelData.type === "wifi" ? qsTr("Wi-Fi") : qsTr("Wired"))
                font.bold: true
            }
            NetworkNote {
                text: Number(device.modelData.state) === 100 ? qsTr("Connected") : Number(device.modelData.state) >= 40 && Number(device.modelData.state) < 100 ? qsTr("Connecting…") : qsTr("Disconnected")
            }
            HarborButton {
                text: qsTr("Disconnect")
                enabled: !NetworkSettings.busy && Number(device.modelData.state) >= 40
                onClicked: NetworkSettings.disconnectDevice(device.modelData.path)
            }
        }
    }
    HarborLabel {
        text: qsTr("Nearby Wi-Fi networks")
        font.bold: true
    }
    Repeater {
        model: NetworkSettings.networks
        delegate: NetworkCard {
            id: ap
            required property var modelData
            HarborLabel {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: ap.modelData.name || qsTr("Hidden network")
                font.bold: true
            }
            NetworkNote {
                text: Number(ap.modelData.strength) + "% · " + (ap.modelData.enterprise ? qsTr("Enterprise") : ap.modelData.secured ? qsTr("Secured") : qsTr("Open"))
            }
            HarborButton {
                text: qsTr("Connect")
                enabled: !NetworkSettings.busy
                onClicked: {
                    if (ap.modelData.enterprise)
                        root.enterprise(ap.modelData.name);
                    else if (ap.modelData.secured)
                        root.selectedAp = ap.modelData.path;
                    else
                        NetworkSettings.connectWifi(ap.modelData.path, "");
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                visible: root.selectedAp === ap.modelData.path
                NetworkNote {
                    visible: ap.modelData.enterprise
                    text: qsTr("Use an existing 802.1X profile from Saved connections for enterprise Wi-Fi.")
                }
                HarborField {
                    id: password
                    Layout.fillWidth: true
                    visible: !ap.modelData.enterprise
                    placeholderText: qsTr("Wi-Fi password")
                    Accessible.name: placeholderText
                    echoMode: TextInput.Password
                }
                RowLayout {
                    HarborButton {
                        visible: !ap.modelData.enterprise
                        text: qsTr("Join network")
                        enabled: !NetworkSettings.busy
                        onClicked: {
                            NetworkSettings.connectWifi(ap.modelData.path, password.text);
                            password.text = "";
                        }
                    }
                    HarborButton {
                        text: qsTr("Cancel")
                        onClicked: {
                            password.text = "";
                            root.selectedAp = "";
                        }
                    }
                }
            }
        }
    }
    HarborLabel {
        text: qsTr("Saved connections")
        font.bold: true
    }
    Repeater {
        model: NetworkSettings.profiles
        delegate: NetworkCard {
            id: connection
            required property var modelData
            HarborLabel {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: connection.modelData.name
                font.bold: true
            }
            NetworkNote {
                text: connection.modelData.type
            }
            Flow {
                Layout.fillWidth: true
                spacing: 8
                HarborButton {
                    text: qsTr("Connect")
                    enabled: !NetworkSettings.busy
                    onClicked: NetworkSettings.activate(connection.modelData.path)
                }
                HarborButton {
                    text: qsTr("Edit connection")
                    enabled: !NetworkSettings.busy
                    onClicked: root.edit(connection.modelData.path)
                }
            }
        }
    }
    HarborButton {
        text: qsTr("New enterprise Wi-Fi")
        enabled: NetworkSettings.available && !NetworkSettings.busy
        onClicked: root.enterprise("")
    }
    HarborButton {
        text: qsTr("New wired connection")
        enabled: NetworkSettings.available && !NetworkSettings.busy
        onClicked: root.edit("")
    }
    ConnectionEditor {
        visible: root.editing
        page: root
    }
    NetworkCard {
        HarborLabel {
            text: qsTr("Import VPN")
            font.bold: true
        }
        NetworkNote {
            text: qsTr("Import a local OpenVPN or WireGuard configuration. The matching NetworkManager plugin must be installed. Imported profiles are restricted to your user.")
        }
        ComboBox {
            id: vpnType
            Layout.fillWidth: true
            model: ["OpenVPN", "WireGuard"]
            Accessible.name: qsTr("VPN type")
        }
        HarborField {
            id: vpnFile
            Layout.fillWidth: true
            placeholderText: qsTr("Configuration file path")
            Accessible.name: placeholderText
        }
        HarborButton {
            text: qsTr("Import")
            enabled: NetworkSettings.available && !NetworkSettings.busy && vpnFile.text.length > 0
            onClicked: NetworkSettings.importVpn(vpnFile.text, vpnType.currentIndex === 0 ? "openvpn" : "wireguard")
        }
    }
    NetworkNote {
        visible: NetworkSettings.busy
        text: qsTr("Updating network settings…")
    }
    NetworkNote {
        visible: NetworkSettings.error.length > 0
        text: NetworkSettings.error
        color: Prefs.dark ? "#ffaaa4" : "#b22b24"
    }
}
