import QtQuick
Item {
 Desktop{anchors.fill:parent}
 MenuBar{anchors.top:parent.top;width:parent.width;height:32}
 Settings{width:920;height:650;anchors.centerIn:parent}
 Dock{width:720;height:78;anchors.horizontalCenter:parent.horizontalCenter;anchors.bottom:parent.bottom;anchors.bottomMargin:12}
}
