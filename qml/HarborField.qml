import QtQuick
import QtQuick.Controls

TextField {
    color: Prefs.dark ? "#eeeeef" : "#26262a"
    placeholderTextColor: Prefs.dark ? "#a1a1a9" : "#85858d"
    selectionColor: Prefs.accent
    background: Rectangle {
        radius: 8
        color: Prefs.dark ? "#39393e" : "#ffffff"
        border.color: parent.activeFocus ? Prefs.accent : "#30909098"
    }
}
