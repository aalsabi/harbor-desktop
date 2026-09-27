pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

NetworkCard {
    id: editor
    required property Item page
    HarborLabel {
        text: qsTr("Connection settings")
        font.bold: true
    }
    HarborField {
        Layout.fillWidth: true
        text: editor.page.draft.name || ""
        placeholderText: qsTr("Connection name")
        Accessible.name: placeholderText
        onTextEdited: editor.page.stage("name", text)
    }
    HarborField {
        Layout.fillWidth: true
        visible: editor.page.editingPath === ""
        text: editor.page.draft.interface || ""
        placeholderText: qsTr("Interface (optional, e.g. eth0)")
        Accessible.name: placeholderText
        onTextEdited: editor.page.stage("interface", text)
    }
    HarborField {
        Layout.fillWidth: true
        visible: editor.page.editingPath === "" && editor.page.draft.type === "802-11-wireless"
        text: editor.page.draft.ssid || ""
        placeholderText: qsTr("Wi-Fi network name (SSID)")
        Accessible.name: placeholderText
        onTextEdited: editor.page.stage("ssid", text)
    }
    CheckBox {
        visible: editor.page.draft.type === "802-3-ethernet" && !editor.page.draft.enterprise
        text: qsTr("Configure 802.1X authentication")
        onClicked: {
            editor.page.stage("enterprise", true);
            editor.page.stage("eap", "peap");
            editor.page.stage("phase2Auth", "mschapv2");
        }
    }
    ColumnLayout {
        visible: !!editor.page.draft.enterprise
        Layout.fillWidth: true
        HarborLabel {
            text: qsTr("Enterprise authentication · 802.1X")
            font.bold: true
        }
        NetworkNote {
            visible: editor.page.draft.enterpriseEditable === false
            text: qsTr("This profile uses advanced authentication managed outside Harbor. Its authentication settings are preserved.")
        }
        ColumnLayout {
            visible: editor.page.draft.enterpriseEditable !== false
            Layout.fillWidth: true
            ComboBox {
                Layout.fillWidth: true
                model: ["PEAP", "TTLS", "TLS"]
                currentIndex: ["peap", "ttls", "tls"].indexOf(editor.page.draft.eap || "peap")
                Accessible.name: qsTr("EAP authentication")
                onActivated: {
                    editor.page.stage("eap", ["peap", "ttls", "tls"][currentIndex]);
                    editor.page.stage("phase2Auth", "mschapv2");
                }
            }
            Repeater {
                model: [
                    {
                        key: "identity",
                        en: QT_TR_NOOP("Identity")
                    },
                    {
                        key: "anonymousIdentity",
                        en: QT_TR_NOOP("Anonymous identity (optional)")
                    },
                    {
                        key: "domainSuffixMatch",
                        en: QT_TR_NOOP("Authentication server domain")
                    },
                    {
                        key: "caCert",
                        en: QT_TR_NOOP("CA certificate file path")
                    }
                ]
                delegate: HarborField {
                    required property var modelData
                    Layout.fillWidth: true
                    text: editor.page.draft[modelData.key] || ""
                    placeholderText: qsTr(modelData.en)
                    Accessible.name: placeholderText
                    onTextEdited: editor.page.stage(modelData.key, text)
                }
            }
            ComboBox {
                visible: editor.page.draft.eap !== "tls"
                Layout.fillWidth: true
                model: editor.page.draft.eap === "ttls" ? ["mschapv2", "pap", "chap", "mschap"] : ["mschapv2", "gtc"]
                currentIndex: Math.max(0, model.indexOf(editor.page.draft.phase2Auth || "mschapv2"))
                Accessible.name: qsTr("Inner authentication")
                onActivated: editor.page.stage("phase2Auth", model[currentIndex])
            }
            HarborField {
                visible: editor.page.draft.eap !== "tls"
                Layout.fillWidth: true
                text: editor.page.draft.eapPassword || ""
                placeholderText: qsTr("Password (blank keeps saved password or asks on connect)")
                Accessible.name: placeholderText
                echoMode: TextInput.Password
                onTextEdited: editor.page.stage("eapPassword", text)
            }
            Repeater {
                model: [
                    {
                        key: "clientCert",
                        en: QT_TR_NOOP("Client certificate file path")
                    },
                    {
                        key: "privateKey",
                        en: QT_TR_NOOP("Private key file path")
                    }
                ]
                delegate: HarborField {
                    required property var modelData
                    visible: editor.page.draft.eap === "tls"
                    Layout.fillWidth: true
                    text: editor.page.draft[modelData.key] || ""
                    placeholderText: qsTr(modelData.en)
                    Accessible.name: placeholderText
                    onTextEdited: editor.page.stage(modelData.key, text)
                }
            }
            HarborField {
                visible: editor.page.draft.eap === "tls"
                Layout.fillWidth: true
                text: editor.page.draft.privateKeyPassword || ""
                placeholderText: qsTr("Private key password (blank to keep)")
                Accessible.name: placeholderText
                echoMode: TextInput.Password
                onTextEdited: editor.page.stage("privateKeyPassword", text)
            }
            NetworkNote {
                text: qsTr("Use the CA certificate and server domain provided by your administrator. Certificate verification is required. Keep certificate files in a permanent location. New credentials are saved by NetworkManager; new profiles are restricted to your user.")
            }
        }
    }
    Repeater {
        model: ["ipv4", "ipv6"]
        delegate: ColumnLayout {
            id: ip
            required property string modelData
            Layout.fillWidth: true
            spacing: 8
            HarborLabel {
                text: ip.modelData.toUpperCase()
                font.bold: true
            }
            ComboBox {
                Layout.fillWidth: true
                model: [qsTr("Automatic"), qsTr("Manual"), qsTr("Disabled")]
                currentIndex: ["auto", "manual", "disabled"].indexOf(editor.page.draft[ip.modelData + "Method"] || "auto")
                Accessible.name: ip.modelData + qsTr(" addressing")
                onActivated: editor.page.stage(ip.modelData + "Method", ["auto", "manual", "disabled"][currentIndex])
            }
            HarborField {
                Layout.fillWidth: true
                visible: editor.page.draft[ip.modelData + "Method"] === "manual"
                text: editor.page.draft[ip.modelData + "Addresses"] || ""
                placeholderText: ip.modelData === "ipv4" ? "192.168.1.10/24" : "2001:db8::10/64"
                Accessible.name: ip.modelData + qsTr(" addresses and prefixes")
                onTextEdited: editor.page.stage(ip.modelData + "Addresses", text)
            }
            HarborField {
                Layout.fillWidth: true
                visible: editor.page.draft[ip.modelData + "Method"] === "manual"
                text: editor.page.draft[ip.modelData + "Gateway"] || ""
                placeholderText: qsTr("Gateway (optional)")
                Accessible.name: ip.modelData + placeholderText
                onTextEdited: editor.page.stage(ip.modelData + "Gateway", text)
            }
            CheckBox {
                text: qsTr("Use automatic DNS")
                checked: !editor.page.draft[ip.modelData + "ManualDns"]
                enabled: editor.page.draft[ip.modelData + "Method"] !== "disabled"
                onClicked: editor.page.stage(ip.modelData + "ManualDns", !checked)
            }
            HarborField {
                Layout.fillWidth: true
                enabled: editor.page.draft[ip.modelData + "Method"] !== "disabled"
                text: editor.page.draft[ip.modelData + "Dns"] || ""
                placeholderText: qsTr("DNS servers, separated by commas")
                Accessible.name: ip.modelData + placeholderText
                onTextEdited: editor.page.stage(ip.modelData + "Dns", text)
            }
        }
    }
    ColumnLayout {
        visible: !!editor.page.draft.vpnOpenvpn
        Layout.fillWidth: true
        HarborLabel {
            text: qsTr("VPN sign-in")
            font.bold: true
        }
        HarborField {
            Layout.fillWidth: true
            text: editor.page.draft.vpnUsername || ""
            placeholderText: qsTr("VPN username")
            Accessible.name: placeholderText
            onTextEdited: editor.page.stage("vpnUsername", text)
        }
        HarborField {
            id: vpnPassword
            Layout.fillWidth: true
            placeholderText: qsTr("New password (leave blank to keep)")
            Accessible.name: placeholderText
            echoMode: TextInput.Password
            onTextEdited: editor.page.stage("vpnPassword", text)
        }
        NetworkNote {
            text: qsTr("A new password is saved by NetworkManager with this connection. Imported VPNs are restricted to your user.")
        }
    }
    NetworkNote {
        text: qsTr("Saving changes the profile. Connect again to apply it to the active network.")
    }
    RowLayout {
        HarborButton {
            text: qsTr("Save profile")
            enabled: !NetworkSettings.busy
            onClicked: {
                NetworkSettings.saveProfile(editor.page.editingPath, editor.page.draft);
                editor.page.stage("vpnPassword", "");
                editor.page.stage("eapPassword", "");
                editor.page.stage("privateKeyPassword", "");
                vpnPassword.text = "";
            }
        }
        HarborButton {
            text: qsTr("Close")
            onClicked: {
                editor.page.editing = false;
                editor.page.draft = {};
            }
        }
    }
}
