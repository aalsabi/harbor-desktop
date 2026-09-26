import QtQuick

Item {
    Desktop {
        anchors.fill: parent
    }
    MenuBar {
        anchors.top: parent.top
        width: parent.width
        height: 38
    }
    Repeater {
        model: 10
        delegate: Rectangle {
            required property int index
            width: 1100 + index * 3
            height: 660 + index * 3
            anchors.centerIn: parent
            anchors.verticalCenterOffset: 5
            radius: 16 + index
            color: "#03000000"
        }
    }
    Files {
        width: 1100
        height: 660
        anchors.centerIn: parent
    }
    Dock {
        width: 720
        height: 78
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
    }
}
