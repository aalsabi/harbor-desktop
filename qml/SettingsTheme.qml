pragma Singleton
import QtQuick

// Colours shared by the System Settings window and its sections.
QtObject {
    readonly property color ink: Prefs.dark ? "#eeeef0" : "#252527"
    readonly property color muted: Prefs.dark ? "#a6a6ad" : "#76767c"
    readonly property color card: Prefs.dark ? "#303034" : "#ffffff"
    readonly property color line: Prefs.dark ? "#454549" : "#e2e2e7"
}
