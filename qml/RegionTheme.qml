pragma Singleton
import QtQuick

// Colours and direction shared by the Language & Region page and its dialogs.
QtObject {
    readonly property bool arabic: Prefs.language === "ar"
    readonly property color ink: Prefs.dark ? "#eeeeef" : "#26262a"
    readonly property color muted: Prefs.dark ? "#aaaab2" : "#696971"
    readonly property color line: Prefs.dark ? "#505057" : "#d2d2d8"
}
