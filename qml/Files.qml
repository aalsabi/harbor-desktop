import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id:root;color:Prefs.dark?"#242426":"#ffffff";radius:16;clip:true
 property color ink:Prefs.dark?"#eeeeef":"#262628"
 property color muted:Prefs.dark?"#a8a8ae":"#77777e"
 property color sidebar:Prefs.dark?"#303034":"#ededf0"
 property string lastPath:""
 Component.onCompleted:{lastPath=Browser.path;location.text=Browser.path}
 property var selected:[]
 property var detail:({})
 property int viewMode:0
 property string pendingAction:"mkdir"
 property bool previewVisible:false
 function choose(p,modifiers){if(modifiers&Qt.ControlModifier)selected=selected.includes(p)?selected.filter(x=>x!==p):selected.concat([p]);else selected=[p];detail=Browser.preview(p)}
 function ask(action){pendingAction=action;nameField.text=action==="rename"&&selected.length===1?selected[0].split("/").pop():"";nameDialog.open();nameField.forceActiveFocus()}
 Connections{target:Browser;function onChanged(){if(root.lastPath!==Browser.path){root.detail={};root.lastPath=Browser.path;location.text=Browser.path;}selected=selected.filter(p=>Browser.entries.some(e=>e.path===p));}}
 Shortcut{sequence:"Alt+Left";onActivated:Browser.back()}
 Shortcut{sequence:"Alt+Right";onActivated:Browser.forward()}
 Shortcut{sequence:"Alt+Up";onActivated:Browser.up()}
 Shortcut{sequence:"Ctrl+T";onActivated:Browser.addTab()}
 Shortcut{sequence:"Ctrl+W";onActivated:Browser.closeTab(Browser.tab)}
 Shortcut{sequence:"Ctrl+H";onActivated:Browser.hidden=!Browser.hidden}
 Shortcut{sequence:"Ctrl+C";enabled:fileArea.activeFocus;onActivated:Browser.copy(selected,false)}
 Shortcut{sequence:"Ctrl+X";enabled:fileArea.activeFocus;onActivated:Browser.copy(selected,true)}
 Shortcut{sequence:"Ctrl+V";enabled:fileArea.activeFocus;onActivated:Browser.paste()}
 Shortcut{sequence:"F2";enabled:fileArea.activeFocus&&selected.length===1;onActivated:root.ask("rename")}
 Shortcut{sequence:"Space";enabled:fileArea.activeFocus;onActivated:root.previewVisible=!root.previewVisible}
 Shortcut{sequence:"Ctrl+L";onActivated:{location.forceActiveFocus();location.selectAll()}}
 component Tool:HarborButton {
  id:tool;implicitHeight:30;implicitWidth:32;leftPadding:8;rightPadding:8;font.pixelSize:18
  contentItem:Text{text:tool.text;color:root.ink;font:tool.font;horizontalAlignment:Text.AlignHCenter;verticalAlignment:Text.AlignVCenter}
  background:Rectangle{radius:7;color:parent.prominent?(Prefs.dark?"#626268":"#d9d9df"):parent.hovered?(Prefs.dark?"#48484e":"#e6e6eb"):"transparent"}
 }
 ColumnLayout {anchors.fill:parent;spacing:0
  Item {Layout.fillWidth:true;Layout.preferredHeight:58
   Rectangle{x:0;y:0;width:190;height:parent.height;color:root.sidebar;radius:16}
   Rectangle{x:0;y:18;width:190;height:parent.height-18;color:root.sidebar}
   Rectangle{x:174;y:0;width:16;height:parent.height;color:root.sidebar}
   MouseArea{anchors.fill:parent;onPressed:UI.windowAction("move");onDoubleClicked:UI.windowAction("maximize")}
   Row {x:14;anchors.verticalCenter:parent.verticalCenter;spacing:0
    Repeater{model:["#ff6057","#febc2e","#28c840"];delegate:Button{required property string modelData;required property int index;objectName:"traffic-"+index;width:24;height:28;activeFocusOnTab:true;hoverEnabled:true
     Accessible.name:["Close window","Minimize window","Maximize window"][index]
     contentItem:Item{}
     background:Item{Rectangle{anchors.centerIn:parent;width:12;height:12;radius:6;color:parent.parent.modelData;border.color:parent.parent.activeFocus?"#1684f8":Qt.darker(color,1.12);border.width:parent.parent.activeFocus?2:1}}
     onClicked:UI.windowAction(index===0?"close":index===1?"minimize":"maximize")
     ToolTip.visible:hovered;ToolTip.text:Accessible.name
    }}
   }
   RowLayout {anchors.left:parent.left;anchors.leftMargin:202;anchors.right:parent.right;anchors.rightMargin:16;anchors.verticalCenter:parent.verticalCenter;spacing:12
    Rectangle{implicitWidth:68;implicitHeight:34;radius:17;color:Prefs.dark?"#353538":"#f7f7f9";border.color:Prefs.dark?"#4c4c50":"#e8e8ed"
     Row{anchors.centerIn:parent;Tool{text:"‹";onClicked:Browser.back();Accessible.name:"Back"}Tool{text:"›";onClicked:Browser.forward();Accessible.name:"Forward"}}
    }
    HarborLabel{text:Browser.path.split("/").pop()||"Computer";font.bold:true;font.pixelSize:14;Layout.fillWidth:true;elide:Text.ElideRight}
    Rectangle{implicitWidth:104;implicitHeight:34;radius:17;color:Prefs.dark?"#353538":"#f7f7f9";border.color:Prefs.dark?"#4c4c50":"#e8e8ed"
     Row{anchors.centerIn:parent;spacing:2;Repeater{model:["▦","☰","▥"];delegate:Tool{required property int index;required property string modelData;text:modelData;prominent:root.viewMode===index;onClicked:root.viewMode=index;ToolTip.visible:hovered;ToolTip.text:["Icons","List","Columns"][index];Accessible.name:["Icons","List","Columns"][index]}}}
    }
    Tool{text:"◧";prominent:root.previewVisible;onClicked:root.previewVisible=!root.previewVisible;ToolTip.visible:hovered;ToolTip.text:"Preview";Accessible.name:"Preview"}
    Tool{text:"•••";onClicked:actions.open();Accessible.name:"File actions";Menu{id:actions
     MenuItem{text:"New tab";onTriggered:Browser.addTab()}
     MenuItem{text:"New folder";enabled:!Browser.busy;onTriggered:root.ask("mkdir")}
     MenuItem{text:"Copy";enabled:selected.length>0;onTriggered:Browser.copy(selected,false)}
     MenuItem{text:"Cut";enabled:selected.length>0;onTriggered:Browser.copy(selected,true)}
     MenuItem{text:"Paste";enabled:!Browser.busy;onTriggered:Browser.paste()}
     MenuSeparator{}
     MenuItem{text:"Rename";enabled:selected.length===1&&!Browser.busy;onTriggered:root.ask("rename")}
     MenuItem{text:"Move to Trash";enabled:selected.length>0&&!Browser.busy;onTriggered:trashDialog.open()}
     MenuSeparator{}
     MenuItem{text:"Show hidden files";checkable:true;checked:Browser.hidden;onTriggered:Browser.hidden=!Browser.hidden}
     MenuItem{text:"Refresh";onTriggered:Browser.refresh()}
    }}
    HarborField{Layout.preferredWidth:150;implicitHeight:28;placeholderText:"Search this folder";text:Browser.search;onTextEdited:Browser.search=text;Accessible.name:"Filter this folder"}
   }
  }
  RowLayout {visible:Browser.tabs.length>1;Layout.fillWidth:true;Layout.leftMargin:198;Layout.rightMargin:8
   Flickable{Layout.fillWidth:true;Layout.preferredHeight:36;contentWidth:tabs.width;clip:true
    Row{id:tabs;spacing:4;Repeater{model:Browser.tabs;delegate:Row{required property int index;required property string modelData
     HarborButton{height:32;text:modelData.split("/").pop()||"/";prominent:index===Browser.tab;onClicked:Browser.selectTab(index)}
     HarborButton{height:32;text:"×";visible:Browser.tabs.length>1;onClicked:Browser.closeTab(index)}
    }}}
   }
   HarborButton{text:"+";onClicked:Browser.addTab();Accessible.name:"New tab"}
  }
  RowLayout{Layout.fillWidth:true;Layout.fillHeight:true;spacing:0
   Rectangle {Layout.preferredWidth:190;Layout.fillHeight:true;color:root.sidebar
    ColumnLayout{anchors.fill:parent;anchors.leftMargin:12;anchors.rightMargin:12;anchors.topMargin:12;spacing:4
     HarborLabel{text:"Favorites";color:root.muted;font.pixelSize:11;font.bold:true;Layout.leftMargin:10;Layout.bottomMargin:4}
     ListView{Layout.fillWidth:true;Layout.fillHeight:true;clip:true;model:Browser.places;spacing:3
      delegate:Button{required property var modelData;id:placeButton;width:ListView.view.width;height:31;activeFocusOnTab:true;hoverEnabled:true;Accessible.name:modelData.name
       background:Rectangle{radius:7;color:Browser.path===placeButton.modelData.path?(Prefs.dark?"#52525a":"#dcdce2"):placeButton.hovered?(Prefs.dark?"#414148":"#e4e4e9"):"transparent";border.width:placeButton.activeFocus?2:0;border.color:"#1684f8"}
       contentItem:Row{spacing:10;leftPadding:9
        Text{width:18;height:parent.height;text:placeButton.modelData.path==="/"?"▣":placeButton.modelData.name==="Home"?"⌂":placeButton.modelData.name==="Downloads"?"↓":placeButton.modelData.name==="Pictures"?"▧":placeButton.modelData.name==="Desktop"?"▱":"▤";color:"#248bef";font.pixelSize:21;verticalAlignment:Text.AlignVCenter}
        Text{height:parent.height;width:parent.width-40;text:placeButton.modelData.name;elide:Text.ElideRight;color:root.ink;font.pixelSize:13;font.bold:Browser.path===placeButton.modelData.path;verticalAlignment:Text.AlignVCenter}
       }
       onClicked:Browser.navigate(modelData.path)
      }
      ScrollBar.vertical:ScrollBar{}
     }
    }
   }
   ColumnLayout{Layout.fillWidth:true;Layout.fillHeight:true;Layout.margins:16;spacing:0
    Item{id:fileArea;focus:true;Layout.fillWidth:true;Layout.fillHeight:true
     DropArea{anchors.fill:parent;onDropped:drop=>{if(drop.hasUrls){Browser.drop(drop.urls,(drop.modifiers&Qt.ShiftModifier)!==0);drop.acceptProposedAction()}}}
     GridView{id:grid;anchors.fill:parent;visible:root.viewMode===0;clip:true;model:Browser.entries;cellWidth:132;cellHeight:124
      delegate:FileTile{required property var modelData;width:132;height:124;entry:modelData}
      ScrollBar.vertical:ScrollBar{}
     }
     ListView{id:list;anchors.fill:parent;visible:root.viewMode===1;clip:true;model:Browser.entries;spacing:2
      delegate:FileTile{required property var modelData;width:ListView.view.width;height:44;entry:modelData;compact:true}
      ScrollBar.vertical:ScrollBar{}
     }
     RowLayout{anchors.fill:parent;visible:root.viewMode===2;spacing:8
      ListView{Layout.preferredWidth:150;Layout.fillHeight:true;clip:true;model:Browser.children(Browser.parentPath(Browser.path))
       delegate:HarborButton{required property var modelData;width:ListView.view.width;text:modelData.name;visible:modelData.folder;height:visible?34:0;prominent:modelData.path===Browser.path;onClicked:Browser.navigate(modelData.path)}
       ScrollBar.vertical:ScrollBar{}
      }
      Rectangle{Layout.fillHeight:true;width:1;color:"#456078"}
      ListView{Layout.fillWidth:true;Layout.fillHeight:true;clip:true;model:Browser.entries
       delegate:FileTile{required property var modelData;width:ListView.view.width;height:40;entry:modelData;compact:true}
       ScrollBar.vertical:ScrollBar{}
      }
      ListView{Layout.preferredWidth:160;Layout.fillHeight:true;clip:true;model:selected.length===1?Browser.children(selected[0]):[]
       delegate:HarborButton{required property var modelData;width:ListView.view.width;text:modelData.name;onClicked:root.choose(modelData.path,0);onDoubleClicked:Browser.open(modelData.path)}
       ScrollBar.vertical:ScrollBar{}
      }
     }
     HarborLabel{anchors.centerIn:parent;visible:Browser.entries.length===0;text:Browser.search.length?"No matching items":"This folder is empty";opacity:.65}
    }
   }
   Rectangle {Layout.preferredWidth:230;Layout.fillHeight:true;visible:root.previewVisible;color:Prefs.dark?"#2c2c30":"#f8f8fa"
    ScrollView{anchors.fill:parent;anchors.margins:16;clip:true;contentWidth:availableWidth
     ColumnLayout{width:parent.width;spacing:16
      Image{Layout.fillWidth:true;Layout.preferredHeight:150;source:detail.image||"";fillMode:Image.PreserveAspectFit;asynchronous:true;sourceSize.width:400;sourceSize.height:400;visible:source.toString().length>0}
      HarborLabel{text:detail.name||"Select a file";font.bold:true;wrapMode:Text.Wrap;Layout.fillWidth:true}
      HarborLabel{text:detail.kind||"";wrapMode:Text.Wrap;Layout.fillWidth:true}
      HarborLabel{text:detail.size!==undefined?detail.size.toLocaleString()+" bytes":""}
      HarborLabel{text:detail.modified||"";wrapMode:Text.Wrap;Layout.fillWidth:true;font.pixelSize:11}
      TextArea{text:detail.text||"";readOnly:true;wrapMode:TextEdit.Wrap;Layout.fillWidth:true;visible:text.length>0;selectByMouse:true;font.pixelSize:12;background:null}
     }
    }
   }
  }
  Rectangle {Layout.fillWidth:true;height:1;color:Prefs.dark?"#444448":"#dedee3"}
  RowLayout{Layout.fillWidth:true;Layout.leftMargin:202;Layout.rightMargin:12;Layout.preferredHeight:32
   HarborField{id:location;objectName:"locationField";Layout.fillWidth:true;implicitHeight:26;font.pixelSize:11;background:null;onAccepted:{Browser.navigate(text);text=Browser.path}Accessible.name:"Folder location"}
   BusyIndicator{running:Browser.busy;visible:running;Layout.preferredWidth:18;Layout.preferredHeight:18}
   HarborLabel{text:Browser.error||Browser.entries.length+" items";elide:Text.ElideRight;Layout.maximumWidth:350;font.pixelSize:11;color:root.muted}
  }
  Item{Layout.fillWidth:true;height:5}

 }
 component FileTile: Rectangle {
  id:tile;objectName:"fileTile-"+entry.name;property var entry;property bool compact:false
  color:root.selected.includes(entry.path)?(Prefs.dark?"#315887":"#d7e9ff"):hover.containsMouse?(Prefs.dark?"#36363b":"#f4f4f7"):"transparent";radius:8
  Image{x:compact?8:(parent.width-64)/2;y:compact?8:12;width:compact?26:64;height:width;source:entry.thumbnail||("image://icons/"+(entry.folder?"folder":entry.icon));fillMode:Image.PreserveAspectFit;sourceSize.width:128;sourceSize.height:128;asynchronous:true}
  Text{x:compact?42:5;y:compact?0:82;width:compact?(parent.width>400?parent.width-255:parent.width-50):parent.width-10;height:compact?parent.height:46;text:entry.name+(entry.link?" ↗":"");color:root.ink;font.pixelSize:12;wrapMode:compact?Text.NoWrap:Text.Wrap;elide:Text.ElideRight;maximumLineCount:2;horizontalAlignment:compact?Text.AlignLeft:Text.AlignHCenter;verticalAlignment:Text.AlignVCenter}
  Text{visible:tile.compact&&tile.width>400;anchors.right:parent.right;anchors.rightMargin:8;anchors.verticalCenter:parent.verticalCenter;text:tile.entry.folder?tile.entry.modified:tile.entry.size.toLocaleString()+" B  ·  "+tile.entry.modified;color:root.muted;font.pixelSize:11}
  MouseArea{id:hover;anchors.fill:parent;hoverEnabled:true;acceptedButtons:Qt.LeftButton|Qt.RightButton;drag.target:dragItem
   onClicked:mouse=>{if(mouse.button===Qt.RightButton)context.open()}
   onDoubleClicked:Browser.open(tile.entry.path)
   onPressed:mouse=>{fileArea.forceActiveFocus();if(mouse.button===Qt.LeftButton&&(mouse.modifiers&Qt.ControlModifier))root.choose(tile.entry.path,mouse.modifiers);else if(!root.selected.includes(tile.entry.path))root.choose(tile.entry.path,0)}
  }
  Item{id:dragItem;Drag.active:hover.drag.active;Drag.dragType:Drag.Automatic;Drag.supportedActions:Qt.CopyAction|Qt.MoveAction;Drag.mimeData:({"text/uri-list":root.selected.map(p=>"file://"+p.split("/").map(encodeURIComponent).join("/")).join("\r\n")})}
  Menu{id:context
   MenuItem{text:"Open";onTriggered:Browser.open(tile.entry.path)}
   MenuItem{text:"Copy";onTriggered:Browser.copy(root.selected,false)}
   MenuItem{text:"Cut";onTriggered:Browser.copy(root.selected,true)}
   MenuItem{text:"Rename";enabled:root.selected.length===1;onTriggered:root.ask("rename")}
   MenuItem{text:"Move to Trash";onTriggered:trashDialog.open()}
  }
 }
 MouseArea{width:14;height:14;anchors.right:parent.right;anchors.bottom:parent.bottom;cursorShape:Qt.SizeFDiagCursor;onPressed:UI.windowAction("resize")}
 Dialog{id:nameDialog;anchors.centerIn:parent;modal:true;title:root.pendingAction==="mkdir"?"New folder":"Rename";standardButtons:Dialog.Ok|Dialog.Cancel
  HarborField{id:nameField;width:300;onAccepted:nameDialog.accept()}
  onAccepted:Browser.operate(root.pendingAction,root.selected,nameField.text)
 }
 Dialog{id:trashDialog;anchors.centerIn:parent;modal:true;title:"Move "+root.selected.length+" item(s) to Trash?";standardButtons:Dialog.Ok|Dialog.Cancel
  Label{text:"Files are sent to the system Trash, not permanently deleted."}
  onAccepted:Browser.operate("trash",root.selected)
 }
}
