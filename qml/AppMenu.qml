import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
GlassCard {
 focus:true;Keys.onEscapePressed:UI.dismiss()
 ColumnLayout {anchors.fill:parent;anchors.margins:20
  RowLayout{HarborButton{text:"←";onClicked:GlobalMenu.back()} HarborLabel{text:Windows.activeTitle;elide:Text.ElideRight;Layout.fillWidth:true} HarborButton{text:"×";onClicked:UI.dismiss()}}
  ListView {Layout.fillWidth:true;Layout.fillHeight:true;model:GlobalMenu.items;clip:true;spacing:6
   delegate:HarborButton{required property var modelData;width:ListView.view.width;text:modelData.label+(modelData.submenu?"  ›":"");enabled:modelData.enabled;onClicked:GlobalMenu.trigger(modelData.id)}
  }
 }
}
