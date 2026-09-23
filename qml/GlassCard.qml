import QtQuick
Rectangle {
 color: Prefs.dark ? Qt.rgba(0.17,0.17,0.20,Prefs.opacity) : Qt.rgba(0.96,0.97,1.0,Prefs.opacity*.82)
 Behavior on color { ColorAnimation {duration:Prefs.reduceMotion?0:160} }
 radius: 20; border.width: 1; border.color: Prefs.dark ? "#35ffffff" : "#bbffffff"
}
