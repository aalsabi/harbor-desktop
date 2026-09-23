import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
GlassCard {
 focus:true;Keys.onEscapePressed:UI.dismiss()
 ColumnLayout {anchors.fill:parent;anchors.margins:24;spacing:16
  RowLayout { HarborLabel {text:qsTr("Applications · right-click to pin");font.pixelSize:24;font.bold:true;Layout.fillWidth:true}
HarborButton{text:"×";onClicked:UI.dismiss()} }
  HarborField {id:search;Layout.fillWidth:true;placeholderText:qsTr("Search installed applications…");focus:true;onAccepted:if(grid.count>0)Apps.launch(grid.model[0].id)}
  GridView {id:grid;Layout.fillWidth:true;Layout.fillHeight:true;clip:true;cellWidth:145;cellHeight:110;model:Apps.entries.filter(a=>a.name.toLowerCase().includes(search.text.toLowerCase()))
   delegate:Item {required property var modelData;width:145;height:110
    Column {anchors.centerIn:parent;spacing:8;Image{anchors.horizontalCenter:parent.horizontalCenter;width:42;height:42;source:"image://icons/"+modelData.icon}
HarborLabel{width:130;text:modelData.name;elide:Text.ElideRight;horizontalAlignment:Text.AlignHCenter}}
    MouseArea{anchors.fill:parent;acceptedButtons:Qt.LeftButton|Qt.RightButton;onClicked:function(mouse){if(mouse.button===Qt.RightButton){let pins=Prefs.pins.slice();let i=pins.indexOf(modelData.id);if(i>=0)pins.splice(i,1);else pins.push(modelData.id);Prefs.pins=pins;}else{Apps.launch(modelData.id);UI.dismiss()}}}
    HarborLabel{anchors.top:parent.top;anchors.right:parent.right;text:Prefs.pins.includes(modelData.id)?"●":"";color:"#81ddd0"}
   }
  }
 }
}
