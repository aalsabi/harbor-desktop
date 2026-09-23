import QtQuick
Rectangle {
 color: Prefs.dark ? Qt.rgba(0.055,0.09,0.14,Prefs.opacity) : Qt.rgba(0.96,0.97,0.99,Prefs.opacity)
 Behavior on color { ColorAnimation {duration:Prefs.reduceMotion?0:160} }
 radius: 20; border.width: 1; border.color: Prefs.dark ? "#35ffffff" : "#660c2440"
}
