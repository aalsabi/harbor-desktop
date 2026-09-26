import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    objectName: "defaultsPage"
    spacing: 14
    property color ink: Prefs.dark ? "#eeeef0" : "#252527"
    property color muted: Prefs.dark ? "#a6a6ad" : "#76767c"
    function title(key) {
        return ({
                browser: qsTr("Web browser"),
                mail: qsTr("Email"),
                files: qsTr("File manager"),
                pdf: qsTr("PDF documents"),
                text: qsTr("Text files"),
                images: qsTr("Images")
            })[key] || key;
    }
    Component.onCompleted: DefaultApps.refresh()
    Text {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: root.muted
        font.pixelSize: 13
        text: qsTr("Choose which applications open links and common file types for your account.")
    }
    Repeater {
        model: DefaultApps.roles
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
                Text {
                    text: root.title(modelData.key)
                    font.pixelSize: 14
                    font.bold: true
                    color: root.ink
                }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    color: root.muted
                    font.pixelSize: 12
                    text: modelData.mixed ? qsTr("Different applications are used for these types.") : modelData.currentName ? qsTr("Current: ") + modelData.currentName : qsTr("No default application is set.")
                }
                ComboBox {
                    id: select
                    Layout.fillWidth: true
                    model: modelData.apps
                    textRole: "name"
                    valueRole: "id"
                    currentIndex: modelData.currentIndex
                    enabled: modelData.apps.length > 0
                    displayText: currentIndex >= 0 ? currentText : qsTr("Choose an application")
                    Accessible.name: root.title(modelData.key)
                    onActivated: {
                        if (!DefaultApps.setDefault(modelData.key, currentValue))
                            DefaultApps.refresh();
                        currentIndex = Qt.binding(function () {
                            return modelData.currentIndex;
                        });
                    }
                    contentItem: Text {
                        text: select.displayText
                        color: root.ink
                        font.pixelSize: 13
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        leftPadding: 10
                        rightPadding: 28
                    }
                    background: Rectangle {
                        radius: 6
                        color: Prefs.dark ? "#45454b" : "#fafafa"
                        border.color: select.activeFocus ? Prefs.accent : (Prefs.dark ? "#55555b" : "#d9d9df")
                    }
                }
                Text {
                    visible: modelData.apps.length === 0
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    color: root.muted
                    font.pixelSize: 12
                    text: qsTr("No installed application supports all of these types.")
                }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    color: root.muted
                    font.pixelSize: 11
                    text: modelData.types
                }
            }
        }
    }
    Text {
        Layout.fillWidth: true
        visible: text.length > 0
        text: DefaultApps.message
        wrapMode: Text.WordWrap
        color: root.ink
        font.pixelSize: 13
    }
    HarborButton {
        text: qsTr("Refresh applications")
        onClicked: DefaultApps.refresh()
    }
}
