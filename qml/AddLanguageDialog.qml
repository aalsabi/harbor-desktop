import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RegionDialog {
    id: addDialog
    required property Item page
    property alias searchText: languageSearch.text
    objectName: "addLanguageDialog"
    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(480, parent ? parent.width - 32 : 480)
    height: Math.min(540, parent ? parent.height - 32 : 540)
    modal: true
    closePolicy: Popup.CloseOnEscape
    title: qsTr("Add a preferred language")
    property string selectedCode: ""
    contentItem: ColumnLayout {
        LayoutMirroring.enabled: RegionTheme.arabic
        LayoutMirroring.childrenInherit: true
        spacing: 10
        HarborField {
            id: languageSearch
            objectName: "languageSearch"
            Layout.fillWidth: true
            placeholderText: qsTr("Search languages")
            Accessible.name: placeholderText
        }
        ListView {
            id: availableLanguages
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: Region.languages.filter(function (l) {
                var q = languageSearch.text.toLowerCase();
                return (page.draft.languages || []).indexOf(l.code) < 0 && (l.name + " " + l.nativeName + " " + l.code).toLowerCase().indexOf(q) >= 0;
            })
            ScrollBar.vertical: ScrollBar {}
            delegate: ItemDelegate {
                id: availableLanguage
                required property var modelData
                width: ListView.view.width
                height: 44
                text: page.languageTitle(modelData.code)
                highlighted: addDialog.selectedCode === modelData.code
                contentItem: Text {
                    text: availableLanguage.text
                    color: availableLanguage.highlighted ? "white" : RegionTheme.ink
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    font.pixelSize: 13
                }
                background: Rectangle {
                    radius: 5
                    color: availableLanguage.highlighted ? Prefs.accent : availableLanguage.hovered ? (Prefs.dark ? "#414149" : "#e7e7ed") : "transparent"
                }
                onClicked: addDialog.selectedCode = modelData.code
            }
            HarborLabel {
                anchors.centerIn: parent
                visible: availableLanguages.count === 0
                text: qsTr("No matching languages")
            }
        }
    }
    footer: Frame {
        padding: 16
        background: Item {}
        contentItem: RowLayout {
            spacing: 8
            Item {
                Layout.fillWidth: true
            }
            HarborButton {
                text: qsTr("Cancel")
                onClicked: addDialog.close()
            }
            HarborButton {
                objectName: "confirmAddLanguage"
                text: qsTr("Add")
                prominent: true
                enabled: addDialog.selectedCode.length > 0
                onClicked: {
                    var a = page.draft.languages.slice();
                    a.push(addDialog.selectedCode);
                    page.stage("languages", a);
                    page.selectedLanguage = a.length - 1;
                    addDialog.close();
                }
            }
        }
    }
}
