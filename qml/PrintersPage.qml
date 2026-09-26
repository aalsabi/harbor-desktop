import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    objectName: "printersPage"
    spacing: 14
    property bool initialDiscovery: true
    property string removeTarget: ""
    property string cancelTarget: ""
    property color ink: Prefs.dark ? "#eeeef0" : "#252527"
    property color muted: Prefs.dark ? "#a6a6ad" : "#76767c"
    component Choice: ComboBox {
        Layout.fillWidth: true
        contentItem: Text {
            text: parent.displayText
            color: root.ink
            verticalAlignment: Text.AlignVCenter
            leftPadding: 10
            rightPadding: 24
            elide: Text.ElideRight
        }
        background: Rectangle {
            radius: 6
            color: Prefs.dark ? "#45454b" : "#fafafa"
            border.color: Prefs.dark ? "#55555b" : "#d9d9df"
        }
    }
    Component.onCompleted: Printers.refresh()
    Connections {
        target: Printers
        function onChanged() {
            if (root.initialDiscovery && !Printers.busy) {
                root.initialDiscovery = false;
                Printers.discover();
            }
        }
    }
    Connections {
        target: PrinterDrivers
        function onInstalled() {
            if (!Printers.busy)
                Printers.loadDrivers();
        }
    }
    RowLayout {
        Layout.fillWidth: true
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: root.muted
            font.pixelSize: 13
            text: qsTr("Printers and pending jobs from your printing service.")
        }
        HarborButton {
            text: Printers.busy ? qsTr("Loading…") : qsTr("Refresh")
            enabled: !Printers.busy
            onClicked: Printers.refresh()
        }
    }
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: notice.implicitHeight + 28
        radius: 10
        color: Prefs.dark ? "#303034" : "white"
        border.color: Prefs.dark ? "#454549" : "#e2e2e7"
        Text {
            id: notice
            anchors.fill: parent
            anchors.margins: 14
            wrapMode: Text.WordWrap
            color: root.muted
            font.pixelSize: 13
            text: qsTr("Add an IPP printer or manage existing queues. Administrator changes require system authorization. Printing preferences apply to your account.")
        }
    }
    Text {
        Layout.fillWidth: true
        visible: !Printers.available && !Printers.busy
        wrapMode: Text.WordWrap
        color: root.ink
        font.pixelSize: 13
        text: qsTr("Printing tools are unavailable. Install cups-client to use printer settings.")
    }
    Text {
        Layout.fillWidth: true
        visible: Printers.available && !Printers.busy && Printers.devices.length === 0
        wrapMode: Text.WordWrap
        color: root.muted
        font.pixelSize: 13
        text: qsTr("No configured printers were found. Check that the printing service is running and a printer is configured.")
    }
    Repeater {
        model: Printers.devices
        delegate: Rectangle {
            required property var modelData
            Layout.fillWidth: true
            implicitHeight: body.implicitHeight + 28
            radius: 10
            color: Prefs.dark ? "#303034" : "white"
            border.color: Prefs.dark ? "#454549" : "#e2e2e7"
            ColumnLayout {
                id: body
                anchors.fill: parent
                anchors.margins: 14
                spacing: 8
                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        Layout.fillWidth: true
                        text: modelData.name
                        elide: Text.ElideRight
                        color: root.ink
                        font.bold: true
                        font.pixelSize: 14
                    }
                    HarborButton {
                        text: Printers.defaultPrinter === modelData.name ? qsTr("Default") : qsTr("Set as default")
                        enabled: !Printers.busy && Printers.defaultPrinter !== modelData.name
                        onClicked: Printers.setDefault(modelData.name)
                    }
                }
                Text {
                    Layout.fillWidth: true
                    text: modelData.status
                    wrapMode: Text.Wrap
                    color: root.muted
                    font.pixelSize: 12
                }
                Text {
                    text: qsTr("Pending jobs: ") + modelData.jobs.length
                    color: root.muted
                    font.pixelSize: 12
                }
                RowLayout {
                    HarborButton {
                        text: qsTr("Printing options")
                        enabled: !Printers.busy
                        onClicked: Printers.loadOptions(modelData.name)
                    }
                    HarborButton {
                        text: qsTr("Remove printer…")
                        enabled: !Printers.busy
                        onClicked: root.removeTarget = modelData.name
                    }
                }
                Repeater {
                    model: modelData.jobs
                    delegate: RowLayout {
                        required property var modelData
                        Layout.fillWidth: true
                        Text {
                            Layout.fillWidth: true
                            text: modelData.id + " · " + modelData.owner + " · " + modelData.bytes + qsTr(" bytes")
                            wrapMode: Text.Wrap
                            color: root.ink
                            font.pixelSize: 12
                        }
                        HarborButton {
                            text: qsTr("Cancel job…")
                            enabled: !Printers.busy
                            onClicked: root.cancelTarget = modelData.id
                        }
                    }
                }
            }
        }
    }
    Rectangle {
        visible: root.removeTarget.length > 0 || root.cancelTarget.length > 0
        Layout.fillWidth: true
        implicitHeight: confirmBody.implicitHeight + 28
        radius: 10
        color: Prefs.dark ? "#303034" : "white"
        ColumnLayout {
            id: confirmBody
            anchors.fill: parent
            anchors.margins: 14
            Text {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: root.ink
                text: root.removeTarget ? qsTr("Permanently remove this printer queue? ") + root.removeTarget : qsTr("Cancel this print job? ") + root.cancelTarget
            }
            RowLayout {
                HarborButton {
                    text: qsTr("Keep")
                    onClicked: {
                        root.removeTarget = "";
                        root.cancelTarget = "";
                    }
                }
                HarborButton {
                    text: qsTr("Confirm")
                    enabled: !Printers.busy
                    onClicked: {
                        if (root.removeTarget)
                            Printers.removePrinter(root.removeTarget, true);
                        else
                            Printers.cancelJob(root.cancelTarget, true);
                        root.removeTarget = "";
                        root.cancelTarget = "";
                    }
                }
            }
        }
    }
    Rectangle {
        visible: Printers.selectedPrinter.length > 0
        Layout.fillWidth: true
        implicitHeight: optionsBody.implicitHeight + 28
        radius: 10
        color: Prefs.dark ? "#303034" : "white"
        ColumnLayout {
            id: optionsBody
            anchors.fill: parent
            anchors.margins: 14
            spacing: 8
            Text {
                Layout.fillWidth: true
                color: root.ink
                wrapMode: Text.Wrap
                text: qsTr("Personal printing options — ") + Printers.selectedPrinter
            }
            Repeater {
                model: Printers.options
                delegate: ColumnLayout {
                    required property var modelData
                    Layout.fillWidth: true
                    Text {
                        text: modelData.name
                        color: root.ink
                        font.pixelSize: 13
                    }
                    Choice {
                        model: modelData.choices
                        currentIndex: modelData.currentIndex
                        enabled: !Printers.busy
                        onActivated: {
                            Printers.setOption(Printers.selectedPrinter, modelData.key, currentText);
                            currentIndex = Qt.binding(function () {
                                return modelData.currentIndex;
                            });
                        }
                    }
                }
            }
            Text {
                visible: !Printers.busy && Printers.options.length === 0
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: root.muted
                text: qsTr("The printer did not report configurable options.")
            }
        }
    }
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: addBody.implicitHeight + 28
        radius: 10
        color: Prefs.dark ? "#303034" : "white"
        border.color: Prefs.dark ? "#454549" : "#e2e2e7"
        ColumnLayout {
            id: addBody
            anchors.fill: parent
            anchors.margins: 14
            spacing: 10
            Text {
                text: qsTr("Add printer")
                color: root.ink
                font.bold: true
                font.pixelSize: 14
            }
            RowLayout {
                Layout.fillWidth: true
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    color: root.muted
                    text: qsTr("Turn on your printer and connect it to this computer or the same network.")
                }
                HarborButton {
                    text: qsTr("Find printers")
                    enabled: !Printers.busy
                    onClicked: Printers.discover()
                }
            }
            Repeater {
                model: Printers.discovered
                delegate: RowLayout {
                    required property var modelData
                    Layout.fillWidth: true
                    ColumnLayout {
                        Layout.fillWidth: true
                        Text {
                            Layout.fillWidth: true
                            text: modelData.name
                            color: root.ink
                            wrapMode: Text.Wrap
                        }
                        Text {
                            Layout.fillWidth: true
                            text: modelData.uri
                            color: root.muted
                            font.pixelSize: 12
                            wrapMode: Text.Wrap
                        }
                    }
                    HarborButton {
                        text: qsTr("Select")
                        enabled: !Printers.busy
                        onClicked: {
                            printerUri.text = modelData.uri;
                            printerName.text = modelData.name.replace(/[^a-zA-Z0-9_-]/g, "_").substring(0, 60) || "Printer";
                            Printers.selectDevice(modelData.uri);
                            family.currentIndex = Math.max(0, family.model.indexOf(PrinterDrivers.suggestedFamily(modelData.model || modelData.name)));
                        }
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: root.muted
                text: qsTr("Choose a discovered printer, or enter an IPP address below. Driverless printers usually need no extra download. Installed drivers are matched by the printer’s reported model ID when available.")
            }
            HarborField {
                id: printerName
                Layout.fillWidth: true
                placeholderText: qsTr("Printer name")
                Accessible.name: placeholderText
            }
            HarborField {
                id: printerUri
                Layout.fillWidth: true
                placeholderText: "ipp://printer.local/ipp/print"
                Accessible.name: qsTr("Printer IPP address")
            }
            Choice {
                id: driver
                model: Printers.drivers
                textRole: "name"
                valueRole: "id"
                Accessible.name: qsTr("Printer driver")
            }
            RowLayout {
                HarborButton {
                    text: qsTr("Load installed drivers")
                    enabled: !Printers.busy
                    onClicked: Printers.loadDrivers()
                }
                HarborButton {
                    text: qsTr("Add printer")
                    prominent: true
                    enabled: !Printers.busy && printerName.text.length > 0 && printerUri.text.length > 0 && driver.currentIndex >= 0
                    onClicked: Printers.addPrinter(printerName.text, printerUri.text, driver.currentValue)
                }
            }
        }
    }
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: driverBody.implicitHeight + 28
        radius: 10
        color: Prefs.dark ? "#303034" : "white"
        border.color: Prefs.dark ? "#454549" : "#e2e2e7"
        ColumnLayout {
            id: driverBody
            anchors.fill: parent
            anchors.margins: 14
            spacing: 10
            Text {
                text: qsTr("Missing printer driver?")
                color: root.ink
                font.bold: true
            }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: root.muted
                text: qsTr("Find drivers from your distribution’s enabled repositories. Family suggestions are not a guarantee of model compatibility. Review the package description and every dependency change before installing. Some printers need drivers not offered here.")
            }
            RowLayout {
                Layout.fillWidth: true
                Choice {
                    id: family
                    model: ["HP", "Epson", "Brother", "Samsung", "Other"]
                    enabled: !PrinterDrivers.busy
                    Accessible.name: qsTr("Printer family")
                }
                HarborButton {
                    text: qsTr("Find missing drivers")
                    enabled: !PrinterDrivers.busy
                    onClicked: PrinterDrivers.search(family.currentText)
                }
            }
            Repeater {
                model: PrinterDrivers.packages
                delegate: RowLayout {
                    required property var modelData
                    Layout.fillWidth: true
                    Text {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        color: root.ink
                        text: modelData.name + " · " + modelData.version + "\n" + modelData.summary
                    }
                    HarborButton {
                        text: qsTr("Review install…")
                        enabled: !PrinterDrivers.busy
                        onClicked: PrinterDrivers.prepare([modelData.id])
                    }
                }
            }
            Repeater {
                model: PrinterDrivers.preview
                delegate: Text {
                    required property var modelData
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    color: root.ink
                    text: modelData.action + " · " + modelData.name + " · " + modelData.version
                }
            }
            Text {
                visible: PrinterDrivers.readyToInstall
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: root.ink
                text: qsTr("Install these packages and apply all changes shown above? The system may ask for administrator authorization.")
            }
            RowLayout {
                visible: PrinterDrivers.readyToInstall
                HarborButton {
                    text: qsTr("Cancel")
                    enabled: !PrinterDrivers.busy
                    onClicked: PrinterDrivers.discardPreview()
                }
                HarborButton {
                    text: qsTr("Confirm and install")
                    prominent: true
                    enabled: !PrinterDrivers.busy
                    onClicked: PrinterDrivers.install(true)
                }
            }
            ProgressBar {
                visible: PrinterDrivers.busy
                Layout.fillWidth: true
                indeterminate: PrinterDrivers.percentage < 0
                value: PrinterDrivers.percentage / 100
            }
            Text {
                Layout.fillWidth: true
                visible: text.length > 0
                wrapMode: Text.Wrap
                color: root.muted
                text: PrinterDrivers.status
            }
            Text {
                Layout.fillWidth: true
                visible: text.length > 0
                wrapMode: Text.Wrap
                color: root.ink
                text: PrinterDrivers.error
            }
            Text {
                Layout.fillWidth: true
                visible: text.length > 0
                wrapMode: Text.Wrap
                color: root.ink
                text: PrinterDrivers.restart
            }
        }
    }
    Text {
        Layout.fillWidth: true
        visible: text.length > 0
        text: Printers.message
        wrapMode: Text.Wrap
        color: root.ink
        font.pixelSize: 13
    }
}
