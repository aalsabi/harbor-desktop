import QtQuick
Item {
 Image { anchors.fill: parent; source: "qrc:/assets/wallpapers/harbor.svg"; fillMode: Image.PreserveAspectCrop }
 Text { anchors.right: parent.right; anchors.bottom: parent.bottom; anchors.margins: 32; text: "H A R B O R"; color: "#55ffffff"; font.pixelSize: 15; font.letterSpacing: 5 }
}
