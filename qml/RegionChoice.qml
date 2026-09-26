import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ComboBox {
    id: choice
    property var choices: []
    property string selected: ""
    signal chosen(string value)
    model: choices
    textRole: "name"
    valueRole: "code"
    Layout.fillWidth: true
    implicitHeight: 30
    currentIndex: {
        for (var i = 0; i < choices.length; i++)
            if (String(choices[i].code) === selected)
                return i;
        return -1;
    }
    onActivated: chosen(String(currentValue))
    Accessible.name: displayText
    leftPadding: RegionTheme.arabic ? 32 : 10
    rightPadding: RegionTheme.arabic ? 10 : 32
    contentItem: Text {
        text: choice.displayText
        color: RegionTheme.ink
        font.pixelSize: 13
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        horizontalAlignment: RegionTheme.arabic ? Text.AlignRight : Text.AlignLeft
    }
    indicator: Text {
        x: RegionTheme.arabic ? 10 : choice.width - width - 10
        y: (choice.height - height) / 2
        text: "⌄"
        color: RegionTheme.muted
        font.pixelSize: 17
    }
    background: Rectangle {
        radius: 6
        color: Prefs.dark ? "#3c3c42" : "#ffffff"
        border.color: choice.activeFocus ? Prefs.accent : RegionTheme.line
    }
    delegate: ItemDelegate {
        id: option
        required property int index
        required property var modelData
        width: choice.width
        height: 34
        highlighted: choice.highlightedIndex === index
        contentItem: Text {
            text: option.modelData.name
            color: option.highlighted ? "white" : RegionTheme.ink
            font.pixelSize: 13
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            radius: 4
            color: option.highlighted ? Prefs.accent : "transparent"
        }
    }
    popup: Popup {
        y: choice.height + 4
        width: choice.width
        padding: 5
        implicitHeight: Math.min(contentItem.implicitHeight + 10, 280)
        background: Rectangle {
            radius: 7
            color: Prefs.dark ? "#323238" : "#ffffff"
            border.color: RegionTheme.line
        }
        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: choice.popup.visible ? choice.delegateModel : null
            currentIndex: choice.highlightedIndex
            ScrollBar.vertical: ScrollBar {}
        }
    }
}
