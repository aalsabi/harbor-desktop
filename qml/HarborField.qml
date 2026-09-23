import QtQuick
import QtQuick.Controls
TextField {
 color:Prefs.dark?"#edf3fa":"#182e47"
 placeholderTextColor:Prefs.dark?"#899bae":"#64778b"
 selectionColor:"#55bdb2"
 background:Rectangle{radius:10;color:Prefs.dark?"#24364d":"#ffffff";border.color:parent.activeFocus?"#81ddd0":"#406e8b9c"}
}
