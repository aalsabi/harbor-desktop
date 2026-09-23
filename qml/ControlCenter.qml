import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
GlassCard {
 focus: true; Keys.onEscapePressed: UI.dismiss()
 ColumnLayout { anchors.fill: parent; anchors.margins: 18; spacing: 12
  RowLayout { Layout.fillWidth: true; HarborLabel { text: qsTr("Control center"); font.pixelSize: 17; font.bold: true; Layout.fillWidth: true }
 HarborButton { text:"×"; onClicked: UI.dismiss() } }
  GridLayout { columns: 2; Layout.fillWidth: true
   HarborButton { text: System.state.wifiAvailable ? "Wi-Fi · "+System.state.wifi : qsTr("Wi-Fi unavailable"); Layout.fillWidth:true;implicitHeight:58; enabled:System.state.wifiAvailable && !System.busy; prominent:System.state.wifi==="enabled"; onClicked:System.action("wifi",System.state.wifi!=="enabled") }
   HarborButton { text: "Bluetooth"; Layout.fillWidth:true;implicitHeight:58; enabled:System.state.bluetoothAvailable && !System.busy; prominent:(System.state.bluetooth||"").includes("Powered: yes"); onClicked:System.action("bluetooth",!(System.state.bluetooth||"").includes("Powered: yes")) }
  }
  HarborLabel { text: qsTr("Sound")+"   "+(System.state.volume||qsTr("Unavailable")); font.bold:true }
  Slider { Layout.fillWidth:true; from:0; to:1; value:parseFloat((System.state.volume||"Volume: 0").split(" ")[1])||0; enabled:System.state.volumeAvailable&&!System.busy; onMoved: volumeTimer.restart(); Timer {id:volumeTimer;interval:180;onTriggered:System.action("volume",parent.value)} }
  HarborButton {text:qsTr("Toggle mute");enabled:System.state.volumeAvailable&&!System.busy;onClicked:System.action("mute")}
  HarborLabel { text: qsTr("Brightness"); font.bold:true }
  Slider { Layout.fillWidth:true; from:5;to:100;value:70;enabled:System.state.brightnessAvailable&&!System.busy;onMoved:brightnessTimer.restart();Timer{id:brightnessTimer;interval:180;onTriggered:System.action("brightness",Math.round(parent.value))} }
  RowLayout { HarborButton {text:Prefs.dark?qsTr("Light appearance"):qsTr("Dark appearance");onClicked:Prefs.dark=!Prefs.dark}
 HarborButton {text:qsTr("Lock");onClicked:System.action("lock")} }
  HarborLabel { text:System.message;wrapMode:Text.Wrap;Layout.fillWidth:true;color:"#eeaa77";visible:text.length>0 }
  Item { Layout.fillHeight:true }
  HarborButton { text:qsTr("Open system settings");Layout.fillWidth:true;prominent:true;onClicked:UI.open("settings") }
 }
}
