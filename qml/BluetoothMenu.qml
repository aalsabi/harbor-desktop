import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
GlassCard {
 id:root;focus:true;Keys.onEscapePressed:UI.dismiss()
 property bool powered:!!System.state.bluetoothPowered
 property bool busy:!!System.state.bluetoothBusy
 property var devices:System.state.bluetoothDevices||[]
 Component.onCompleted:System.action("bluetooth-scan")
 Component.onDestruction:System.action("bluetooth-stop-scan")
 onPoweredChanged:if(powered)Qt.callLater(function(){System.action("bluetooth-scan")})
 ColumnLayout {anchors.fill:parent;anchors.margins:18;spacing:12
  RowLayout{Layout.fillWidth:true
   HarborLabel{text:"Bluetooth";font.pixelSize:22;font.bold:true;Layout.fillWidth:true}
   Switch{objectName:"bluetooth-power";checked:root.powered;enabled:!!System.state.bluetoothAvailable&&!root.busy;Accessible.name:"Bluetooth power";onToggled:System.action("bluetooth",checked)}
  }
  HarborLabel{objectName:"bluetooth-status";text:!System.state.bluetoothAvailable?qsTr("Bluetooth unavailable"):(System.state.bluetoothStatus||"Off")}
  ListView {Layout.fillWidth:true;Layout.fillHeight:true;clip:true;model:root.devices;spacing:8
   delegate:ColumnLayout{required property var modelData;required property int index;width:ListView.view.width
    HarborLabel{text:modelData.connected?qsTr("Connected devices"):qsTr("Available / saved devices");font.bold:true;visible:index===0||root.devices[index-1].connected!==modelData.connected}
    RowLayout{Layout.fillWidth:true
     HarborLabel{text:modelData.name;textFormat:Text.PlainText;elide:Text.ElideRight;Layout.fillWidth:true}
     HarborButton{text:modelData.connected?qsTr("Disconnect"):qsTr("Connect");enabled:root.powered&&!root.busy;onClicked:System.action(modelData.connected?"bluetooth-disconnect":"bluetooth-connect",modelData.address)}
    }
   }
  }
  HarborLabel{text:qsTr("No devices found");visible:root.powered&&root.devices.length===0}
  HarborLabel{text:System.state.bluetoothError||"";visible:text!=="";wrapMode:Text.Wrap;Layout.fillWidth:true;color:Prefs.dark?"#ffb4ab":"#a32020"}
  HarborButton{text:System.state.bluetoothDiscovering?qsTr("Searching…"):qsTr("Search for devices");enabled:root.powered&&!root.busy&&!System.state.bluetoothDiscovering;onClicked:System.action("bluetooth-scan")}
  HarborButton{text:qsTr("Bluetooth Settings…");onClicked:UI.open("settings:Bluetooth")}
  HarborButton{text:qsTr("Close");onClicked:UI.dismiss()}
 }
}
