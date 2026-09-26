pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    objectName: "storagePage"
    spacing: 16
    readonly property bool arabic: Prefs.language === "ar"
    readonly property color muted: Prefs.dark ? "#aaaab2" : "#696971"
    function size(bytes) {
        var units = ["B", "KiB", "MiB", "GiB", "TiB"];
        var n = Number(bytes), i = 0;
        while (n >= 1024 && i < units.length - 1) {
            n /= 1024;
            ++i;
        }
        return n.toFixed(i ? 1 : 0) + " " + units[i];
    }
    LayoutMirroring.enabled: arabic
    LayoutMirroring.childrenInherit: true
    Component.onCompleted: StorageSettings.refresh()
    HarborLabel {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: root.muted
        text: qsTr("Storage usage for mounted volumes. Available space excludes space reserved by the filesystem.")
    }
    Repeater {
        model: StorageSettings.volumes
        delegate: Rectangle {
            id: volume
            required property var modelData
            Layout.fillWidth: true
            implicitHeight: contents.implicitHeight + 32
            radius: 12
            color: Prefs.dark ? "#29292f" : "#ffffff"
            border.color: Prefs.dark ? "#45454d" : "#dedee4"
            ColumnLayout {
                id: contents
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    margins: 16
                }
                spacing: 10
                HarborLabel {
                    Layout.fillWidth: true
                    font.bold: true
                    text: volume.modelData.name
                    elide: Text.ElideMiddle
                }
                HarborLabel {
                    Layout.fillWidth: true
                    text: volume.modelData.mount + " · " + volume.modelData.filesystem + (volume.modelData.readOnly ? qsTr(" · Read only") : "")
                    color: root.muted
                    wrapMode: Text.WrapAnywhere
                }
                ProgressBar {
                    Layout.fillWidth: true
                    value: volume.modelData.fraction
                    Accessible.name: qsTr("Storage used")
                }
                HarborLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: qsTr("Used: ") + root.size(volume.modelData.used) + qsTr(" of ") + root.size(volume.modelData.total) + qsTr(" · Available: ") + root.size(volume.modelData.available)
                }
                HarborButton {
                    text: qsTr("Open folder")
                    onClicked: StorageSettings.openVolume(volume.modelData.mount)
                }
            }
        }
    }
    HarborLabel {
        visible: StorageSettings.volumes.length === 0 && !StorageSettings.busy
        text: qsTr("No mounted storage volumes found.")
    }
    HarborLabel {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        visible: StorageSettings.error.length > 0
        text: StorageSettings.error
        color: Prefs.dark ? "#ffaaa4" : "#b22b24"
    }
    HarborButton {
        text: StorageSettings.busy ? qsTr("Refreshing…") : qsTr("Refresh")
        enabled: !StorageSettings.busy
        onClicked: StorageSettings.refresh()
    }

    ApplicationStorageSection {
        Layout.fillWidth: true
    }

    HarborLabel {
        text: qsTr("Analyze files")
        font.bold: true
        font.pixelSize: 17
    }
    HarborLabel {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        text: qsTr("Choose a folder to measure allocated disk space. Links and other filesystems are excluded; no files are deleted.")
    }
    HarborField {
        id: scanPath
        Layout.fillWidth: true
        text: StorageSettings.homePath
        Accessible.name: qsTr("Folder to analyze")
    }
    RowLayout {
        HarborButton {
            text: qsTr("Analyze")
            enabled: !StorageSettings.analyzing
            onClicked: StorageSettings.analyze(scanPath.text)
        }
        HarborButton {
            text: qsTr("Cancel")
            enabled: StorageSettings.analyzing
            onClicked: StorageSettings.cancelAnalysis()
        }
    }
    HarborLabel {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        text: StorageSettings.analysisMessage
    }
    // Category names come from StorageSettings; listed here so lupdate extracts them.
    readonly property var categoryNames: [QT_TR_NOOP("Pictures"), QT_TR_NOOP("Videos"), QT_TR_NOOP("Audio"), QT_TR_NOOP("Documents"), QT_TR_NOOP("Archives and packages"), QT_TR_NOOP("Other")]
    Repeater {
        model: StorageSettings.categories
        delegate: HarborLabel {
            required property var modelData
            Layout.fillWidth: true
            text: qsTr(modelData.name) + " · " + root.size(modelData.bytes)
        }
    }
    HarborLabel {
        text: qsTr("Largest files")
        font.bold: true
        visible: StorageSettings.largestFiles.length > 0
    }
    Repeater {
        model: StorageSettings.largestFiles
        delegate: HarborLabel {
            required property var modelData
            Layout.fillWidth: true
            wrapMode: Text.WrapAnywhere
            textFormat: Text.PlainText
            text: root.size(modelData.bytes) + " · " + modelData.path
        }
    }
}
