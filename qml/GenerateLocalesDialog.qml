import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RegionDialog {
    id: generateDialog
    required property Item page
    objectName: "generateLocalesDialog"
    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(500, parent ? parent.width - 32 : 500)
    modal: true
    closePolicy: Region.busy ? Popup.NoAutoClose : Popup.CloseOnEscape
    title: qsTr("Generate regional settings?")
    contentItem: ColumnLayout {
        spacing: 12
        LayoutMirroring.enabled: RegionTheme.arabic
        LayoutMirroring.childrenInherit: true
        RegionNote {
            text: qsTr("The required locales are not generated on this computer. Choose OK to generate them and apply your preferences. Administrator authentication is required.")
        }
        HarborLabel {
            Layout.fillWidth: true
            wrapMode: Text.WrapAnywhere
            text: page.requiredLocales.join(" · ")
            LayoutMirroring.enabled: false
        }
        RowLayout {
            visible: Region.busy
            BusyIndicator {
                running: Region.busy
                implicitWidth: 28
                implicitHeight: 28
            }
            RegionNote {
                text: qsTr("Waiting for authentication or generating locales…")
            }
        }
        RegionNote {
            objectName: "generationError"
            visible: page.status.length > 0
            text: page.status
        }
    }
    footer: Item {
        implicitHeight: 60
        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 8
            Item {
                Layout.fillWidth: true
            }
            HarborButton {
                objectName: "cancelGeneration"
                text: qsTr("Cancel")
                enabled: !Region.busy
                onClicked: generateDialog.close()
            }
            HarborButton {
                objectName: "confirmGeneration"
                text: qsTr("OK")
                prominent: true
                enabled: !Region.busy
                onClicked: {
                    page.status = "";
                    Region.generateAndApply(page.generationDraft);
                }
            }
        }
    }
}
