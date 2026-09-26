import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    objectName: "updatesPage"
    spacing: 14
    property color ink: Prefs.dark ? "#eeeef0" : "#252527"
    property color muted: Prefs.dark ? "#a6a6ad" : "#76767c"
    property var selectedIds: []
    function choose(id, value) {
        let ids = selectedIds.slice();
        let i = ids.indexOf(id);
        if (value && i < 0)
            ids.push(id);
        else if (!value && i >= 0)
            ids.splice(i, 1);
        selectedIds = ids;
        SoftwareUpdate.discardPreview();
        confirmInstall.checked = false;
    }
    function action(info) {
        return info === 13 ? qsTr("Remove") : info === 15 ? qsTr("Replace") : info === 12 ? qsTr("Install") : info === 11 ? qsTr("Update") : qsTr("Change");
    }
    component Card: Rectangle {
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
            spacing: 10
        }
    }
    component Caption: Text {
        Layout.fillWidth: true
        color: root.ink
        font.pixelSize: 13
        wrapMode: Text.WordWrap
    }
    Caption {
        text: qsTr("Review updates from your distribution. Installation starts only after a dependency preview and your confirmation.")
        color: root.muted
    }
    RowLayout {
        Layout.fillWidth: true
        HarborButton {
            text: qsTr("Check updates")
            enabled: !SoftwareUpdate.busy
            onClicked: {
                root.selectedIds = [];
                confirmInstall.checked = false;
                SoftwareUpdate.checkUpdates(false);
            }
        }
        HarborButton {
            text: qsTr("Refresh package information")
            enabled: !SoftwareUpdate.busy
            onClicked: {
                root.selectedIds = [];
                confirmInstall.checked = false;
                SoftwareUpdate.checkUpdates(true);
            }
        }
    }
    Caption {
        visible: SoftwareUpdate.status.length > 0
        text: SoftwareUpdate.status
    }
    ProgressBar {
        Layout.fillWidth: true
        visible: SoftwareUpdate.busy
        from: 0
        to: 100
        value: SoftwareUpdate.percentage
        indeterminate: SoftwareUpdate.percentage < 0
    }
    Card {
        visible: SoftwareUpdate.packages.length > 0
        RowLayout {
            Layout.fillWidth: true
            Caption {
                text: qsTr("Available updates")
                font.bold: true
            }
            HarborButton {
                text: qsTr("Select all")
                enabled: !SoftwareUpdate.busy
                onClicked: {
                    root.selectedIds = SoftwareUpdate.packages.filter(p => !p.blocked).map(p => p.id);
                    SoftwareUpdate.discardPreview();
                    confirmInstall.checked = false;
                }
            }
        }
        Repeater {
            model: SoftwareUpdate.packages
            delegate: ColumnLayout {
                required property var modelData
                Layout.fillWidth: true
                spacing: 3
                CheckBox {
                    id: choice
                    Layout.fillWidth: true
                    enabled: !SoftwareUpdate.busy && !modelData.blocked
                    checked: root.selectedIds.indexOf(modelData.id) >= 0
                    onClicked: root.choose(modelData.id, checked)
                    text: modelData.name + " · " + modelData.version + (modelData.security ? qsTr(" · Security update") : "") + (modelData.blocked ? qsTr(" · Blocked") : "")
                    contentItem: Text {
                        text: choice.text
                        color: root.ink
                        leftPadding: choice.indicator.width + 8
                        wrapMode: Text.WordWrap
                        font.pixelSize: 13
                    }
                }
                Caption {
                    text: modelData.summary
                    color: root.muted
                    font.pixelSize: 12
                }
            }
        }
        HarborButton {
            text: qsTr("Review selected updates")
            prominent: true
            enabled: !SoftwareUpdate.busy && root.selectedIds.length > 0
            onClicked: {
                confirmInstall.checked = false;
                SoftwareUpdate.prepare(root.selectedIds);
            }
        }
    }
    Card {
        visible: SoftwareUpdate.preview.length > 0
        Caption {
            text: qsTr("Dependency preview")
            font.bold: true
        }
        Caption {
            text: qsTr("This includes dependencies and any packages that will be removed or replaced. Package availability can change before installation.")
            color: root.muted
        }
        Repeater {
            model: SoftwareUpdate.preview
            delegate: Caption {
                required property var modelData
                text: root.action(modelData.info) + " · " + modelData.name + " · " + modelData.version
            }
        }
        CheckBox {
            id: confirmInstall
            Layout.fillWidth: true
            visible: SoftwareUpdate.readyToInstall
            text: qsTr("I reviewed these changes and want to install them")
            contentItem: Text {
                text: confirmInstall.text
                color: root.ink
                leftPadding: confirmInstall.indicator.width + 8
                wrapMode: Text.WordWrap
                font.pixelSize: 13
            }
        }
        HarborButton {
            text: qsTr("Install trusted updates")
            prominent: true
            enabled: !SoftwareUpdate.busy && SoftwareUpdate.readyToInstall && confirmInstall.checked
            onClicked: {
                SoftwareUpdate.install(true);
                confirmInstall.checked = false;
            }
        }
    }
    Caption {
        visible: SoftwareUpdate.error.length > 0
        text: SoftwareUpdate.error
    }
    Caption {
        visible: SoftwareUpdate.restart.length > 0
        text: SoftwareUpdate.restart
        font.bold: true
    }
    Caption {
        text: qsTr("Unsigned packages, new repository keys, and license agreements are never accepted automatically.")
        color: root.muted
    }
}
