import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    required property Item settings
    Layout.fillWidth: true
    spacing: 18
    property var selectedLayouts: []
    property string selectedShortcut: ""
    property int selectedSource: 0
    function sourceName(code) {
        if (code === "ara")
            return qsTr("Arabic");
        if (code === "us")
            return qsTr("English (US)");
        return ((Keyboard.state.catalog || []).find(x => x.id === code) || ({
                    name: code
                })).name;
    }
    function editSources() {
        selectedLayouts = (Keyboard.state.layouts || []).slice();
        selectedShortcut = Keyboard.state.shortcut || "";
        selectedSource = 0;
        sourceEditor.open();
    }
    SettingsGroup {
        SettingsLabel {
            text: qsTr("Text Input")
            font.bold: true
            font.pixelSize: 16
        }
        SettingRow {
            label: qsTr("Input Sources")
            hint: (Keyboard.state.layouts || []).map(code => root.sourceName(code)).join(" · ")
            HarborButton {
                objectName: "edit-input-sources"
                text: qsTr("Edit…")
                enabled: !Keyboard.busy
                onClicked: root.editSources()
            }
        }
    }
    SettingsNote {
        text: qsTr("Use the input menu in the menu bar to change your typing language.")
    }
    HarborField {
        Layout.fillWidth: true
        placeholderText: qsTr("Type here to test your keyboard…")
    }
    SettingsNote {
        text: Keyboard.message
        visible: text.length > 0
    }
    Popup {
        id: sourceEditor
        parent: root.settings
        anchors.centerIn: parent
        width: Math.min(760, root.settings.width - 40)
        height: Math.min(550, root.settings.height - 40)
        padding: root.settings.height < 620 ? 16 : 22
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape
        background: Rectangle {
            objectName: "input-sources-background"
            radius: 14
            color: SettingsTheme.card
            border.color: SettingsTheme.line
        }
        contentItem: ColumnLayout {
            LayoutMirroring.enabled: Prefs.language === "ar"
            LayoutMirroring.childrenInherit: true
            spacing: root.settings.height < 620 ? 10 : 18
            SettingsLabel {
                text: qsTr("Input Sources")
                font.pixelSize: 21
                font.bold: true
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 22
                Rectangle {
                    Layout.preferredWidth: 230
                    Layout.fillHeight: true
                    radius: 8
                    color: Prefs.dark ? "#252529" : "#f5f5f7"
                    border.color: SettingsTheme.line
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 5
                        ListView {
                            id: configuredSources
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            spacing: 3
                            model: root.selectedLayouts
                            delegate: Button {
                                required property string modelData
                                required property int index
                                objectName: "configured-input-" + modelData
                                width: ListView.view.width
                                height: 52
                                onClicked: root.selectedSource = index
                                background: Rectangle {
                                    radius: 6
                                    color: root.selectedSource === index ? Prefs.accent : parent.hovered ? SettingsTheme.line : "transparent"
                                }
                                contentItem: RowLayout {
                                    spacing: 10
                                    Text {
                                        text: modelData === "ara" ? "ع" : modelData.toUpperCase()
                                        font.pixelSize: 18
                                        color: root.selectedSource === index ? "white" : SettingsTheme.ink
                                        Layout.preferredWidth: 30
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                    Text {
                                        text: root.sourceName(modelData)
                                        color: root.selectedSource === index ? "white" : SettingsTheme.ink
                                        font.pixelSize: 13
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                        SettingsDivider {}
                        RowLayout {
                            spacing: 4
                            HarborButton {
                                objectName: "add-input"
                                text: "+"
                                Accessible.name: qsTr("Add input source")
                                enabled: root.selectedLayouts.length < 4
                                onClicked: {
                                    candidateSearch.text = "";
                                    sourceChooser.candidate = "";
                                    sourceChooser.open();
                                }
                            }
                            HarborButton {
                                objectName: "remove-input"
                                text: "−"
                                Accessible.name: qsTr("Remove input source")
                                enabled: root.selectedLayouts.length > 1
                                onClicked: {
                                    root.selectedLayouts = root.selectedLayouts.filter((x, i) => i !== root.selectedSource);
                                    root.selectedSource = Math.max(0, Math.min(root.selectedSource, root.selectedLayouts.length - 1));
                                }
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: root.settings.height < 620 ? 8 : 16
                    Rectangle {
                        Layout.alignment: Qt.AlignHCenter
                        width: root.settings.height < 620 ? 56 : 76
                        height: width
                        radius: 14
                        color: Prefs.dark ? "#45454b" : "#f0f0f5"
                        border.color: SettingsTheme.line
                        Text {
                            anchors.centerIn: parent
                            text: (root.selectedLayouts[root.selectedSource] || "") === "ara" ? "ع" : (root.selectedLayouts[root.selectedSource] || "").toUpperCase()
                            font.pixelSize: 32
                            color: SettingsTheme.ink
                        }
                    }
                    SettingsLabel {
                        text: root.sourceName(root.selectedLayouts[root.selectedSource] || "")
                        font.pixelSize: 18
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                    SettingsNote {
                        text: root.selectedSource === 0 ? qsTr("Default input source") : qsTr("Available from the input menu")
                        horizontalAlignment: Text.AlignHCenter
                    }
                    HarborButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Make Default")
                        enabled: root.selectedSource > 0
                        onClicked: {
                            let v = root.selectedLayouts.slice();
                            let selected = v.splice(root.selectedSource, 1)[0];
                            v.unshift(selected);
                            root.selectedLayouts = v;
                            root.selectedSource = 0;
                        }
                    }
                    Item {
                        Layout.fillHeight: true
                    }
                    SettingsNote {
                        text: qsTr("Add up to four input sources. The first source is your default.")
                    }
                }
            }
            SettingsDivider {}
            SettingRow {
                label: qsTr("Switch input source")
                SettingsSelect {
                    objectName: "input-shortcut"
                    Layout.preferredWidth: 190
                    model: [qsTr("None"), "Alt + Shift", "Ctrl + Shift", "Super + Space", "Ctrl + Space"]
                    property var values: ["", "grp:alt_shift_toggle", "grp:ctrl_shift_toggle", "grp:win_space_toggle", "grp:ctrl_space_toggle"]
                    currentIndex: Math.max(0, values.indexOf(root.selectedShortcut))
                    onActivated: root.selectedShortcut = values[currentIndex]
                }
            }
            SettingsNote {
                text: qsTr("Super is the Windows or Command key. Existing layout variants are preserved.")
            }
            RowLayout {
                Layout.fillWidth: true
                Item {
                    Layout.fillWidth: true
                }
                HarborButton {
                    objectName: "cancel-input-sources"
                    text: qsTr("Cancel")
                    onClicked: sourceEditor.close()
                }
                HarborButton {
                    objectName: "done-input-sources"
                    text: qsTr("Done")
                    prominent: true
                    enabled: !Keyboard.busy && root.selectedLayouts.length > 0
                    onClicked: {
                        Keyboard.apply(root.selectedLayouts, root.selectedShortcut);
                        sourceEditor.close();
                    }
                }
            }
        }
    }
    Popup {
        id: sourceChooser
        parent: root.settings
        anchors.centerIn: parent
        width: Math.min(480, root.settings.width - 60)
        height: Math.min(450, root.settings.height - 60)
        padding: 22
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape
        property string candidate: ""
        property var matches: (Keyboard.state.catalog || []).filter(x => (x.name + " " + x.id + " " + root.sourceName(x.id)).toLowerCase().includes(candidateSearch.text.toLowerCase()))
        background: Rectangle {
            radius: 14
            color: SettingsTheme.card
            border.color: SettingsTheme.line
        }
        contentItem: ColumnLayout {
            LayoutMirroring.enabled: Prefs.language === "ar"
            LayoutMirroring.childrenInherit: true
            spacing: 14
            SettingsLabel {
                text: qsTr("Add Input Source")
                font.bold: true
                font.pixelSize: 19
            }
            HarborField {
                id: candidateSearch
                objectName: "input-source-search"
                Layout.fillWidth: true
                placeholderText: qsTr("Search languages")
            }
            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                spacing: 3
                model: sourceChooser.matches
                delegate: Button {
                    required property var modelData
                    objectName: "input-candidate-" + modelData.id
                    width: ListView.view.width
                    height: 42
                    enabled: !root.selectedLayouts.includes(modelData.id)
                    onClicked: sourceChooser.candidate = modelData.id
                    background: Rectangle {
                        radius: 6
                        color: sourceChooser.candidate === parent.modelData.id ? Prefs.accent : parent.hovered ? SettingsTheme.line : "transparent"
                    }
                    contentItem: Text {
                        text: root.sourceName(parent.modelData.id) + (root.selectedLayouts.includes(parent.modelData.id) ? " ✓" : "")
                        color: sourceChooser.candidate === parent.modelData.id ? "white" : parent.enabled ? SettingsTheme.ink : SettingsTheme.muted
                        font.pixelSize: 14
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                }
            }
            SettingsNote {
                visible: sourceChooser.matches.length === 0
                text: qsTr("No input sources found")
            }
            RowLayout {
                Item {
                    Layout.fillWidth: true
                }
                HarborButton {
                    text: qsTr("Cancel")
                    onClicked: sourceChooser.close()
                }
                HarborButton {
                    objectName: "confirm-add-input"
                    text: qsTr("Add")
                    prominent: true
                    enabled: sourceChooser.candidate.length > 0 && root.selectedLayouts.length < 4 && !root.selectedLayouts.includes(sourceChooser.candidate)
                    onClicked: {
                        root.selectedLayouts = root.selectedLayouts.concat([sourceChooser.candidate]);
                        root.selectedSource = root.selectedLayouts.length - 1;
                        sourceChooser.close();
                    }
                }
            }
        }
    }
}
