import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
GlassCard {
 id:root;focus:true;Keys.onEscapePressed:UI.dismiss()
 LayoutMirroring.enabled:Prefs.language==="ar";LayoutMirroring.childrenInherit:true
 Connections{target:Keyboard;function onSelectionFinished(success){if(success)UI.dismiss()}}
 ColumnLayout{anchors.fill:parent;anchors.margins:12;spacing:6
  RowLayout{HarborLabel{text:qsTr("Input Sources");font.bold:true;Layout.fillWidth:true}HarborButton{text:"×";Accessible.name:qsTr("Close");onClicked:UI.dismiss()}}
  ListView{Layout.fillWidth:true;Layout.fillHeight:true;model:Keyboard.activeLayouts;clip:true;spacing:3
   delegate:HarborButton{required property var modelData;objectName:"input-source-"+modelData.index;width:ListView.view.width;implicitHeight:40
    text:(Keyboard.activeIndex===modelData.index?"✓  ":"    ")+modelData.label+"  "+modelData.name+(modelData.variant?" ("+modelData.variant+")":"")
    prominent:Keyboard.activeIndex===modelData.index;onClicked:Keyboard.selectLayout(modelData.index)
   }
  }
  HarborLabel{visible:!Keyboard.available;text:qsTr("Input sources unavailable");Layout.fillWidth:true;wrapMode:Text.WordWrap}
  HarborLabel{visible:Keyboard.message.length>0;text:Keyboard.message;Layout.fillWidth:true;wrapMode:Text.WordWrap;font.pixelSize:11}
  Rectangle{height:1;Layout.fillWidth:true;color:Prefs.dark?"#50505a":"#dadade"}
  HarborButton{text:qsTr("Keyboard Settings…");Layout.fillWidth:true;onClicked:UI.open("settings:Keyboard")}
 }
 Component.onCompleted:Keyboard.refreshActive()
}
