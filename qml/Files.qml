import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id:root;color:Prefs.dark?"#111d2c":"#f7f9fc"
 property string lastPath:""
 Component.onCompleted:{lastPath=Browser.path;location.text=Browser.path}
 property var selected:[]
 property var detail:({})
 property int viewMode:0
 property string pendingAction:"mkdir"
 property bool previewVisible:true
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
 ColumnLayout {anchors.fill:parent;spacing:0
  Rectangle {Layout.fillWidth:true;Layout.preferredHeight:64;color:Prefs.dark?"#1a2a3f":"#e7edf5"
   RowLayout {anchors.fill:parent;anchors.margins:12;spacing:8
    HarborLabel{text:"◈  Harbor Files";font.bold:true;font.pixelSize:16;Layout.rightMargin:12}
    HarborButton{text:"‹";onClicked:Browser.back();Accessible.name:"Back"}
    HarborButton{text:"›";onClicked:Browser.forward();Accessible.name:"Forward"}
    HarborButton{text:"↑";onClicked:Browser.up();Accessible.name:"Parent folder"}
    Item{Layout.fillWidth:true}
    Repeater{model:["▦ Icons","☷ List","▥ Columns"];delegate:HarborButton{required property int index;required property string modelData;text:modelData;prominent:root.viewMode===index;onClicked:root.viewMode=index}}
    HarborButton{text:"Preview";prominent:root.previewVisible;onClicked:root.previewVisible=!root.previewVisible}
   }
  }
  RowLayout {Layout.fillWidth:true;Layout.margins:8;spacing:8
   HarborField{id:location;objectName:"locationField";Layout.fillWidth:true;onAccepted:{Browser.navigate(text);text=Browser.path}Accessible.name:"Folder location"}
   HarborField{Layout.preferredWidth:210;placeholderText:"Filter this folder";text:Browser.search;onTextEdited:Browser.search=text;Accessible.name:"Filter this folder"}
  }
  RowLayout {Layout.fillWidth:true;Layout.leftMargin:8;Layout.rightMargin:8
   Flickable{Layout.fillWidth:true;Layout.preferredHeight:36;contentWidth:tabs.width;clip:true
    Row{id:tabs;spacing:4;Repeater{model:Browser.tabs;delegate:Row{required property int index;required property string modelData
     HarborButton{height:32;text:modelData.split("/").pop()||"/";prominent:index===Browser.tab;onClicked:Browser.selectTab(index)}
     HarborButton{height:32;text:"×";visible:Browser.tabs.length>1;onClicked:Browser.closeTab(index)}
    }}}
   }
   HarborButton{text:"+";onClicked:Browser.addTab();Accessible.name:"New tab"}
  }
  RowLayout{Layout.fillWidth:true;Layout.fillHeight:true;spacing:0
   Rectangle {Layout.preferredWidth:190;Layout.fillHeight:true;color:Prefs.dark?"#1b2b40":"#e8eef6"
    ColumnLayout{anchors.fill:parent;anchors.margins:12
     HarborLabel{text:"LOCATIONS";font.pixelSize:11;opacity:.65}
     ListView{Layout.fillWidth:true;Layout.fillHeight:true;clip:true;model:Browser.places;spacing:4
      delegate:HarborButton{required property var modelData;width:ListView.view.width;text:modelData.name;prominent:Browser.path===modelData.path;onClicked:Browser.navigate(modelData.path)}
      ScrollBar.vertical:ScrollBar{}
     }
     HarborButton{text:"Hidden files";prominent:Browser.hidden;onClicked:Browser.hidden=!Browser.hidden}
    }
   }
   ColumnLayout{Layout.fillWidth:true;Layout.fillHeight:true;Layout.margins:10
    RowLayout{Layout.fillWidth:true
     HarborButton{text:"New folder";enabled:!Browser.busy;onClicked:root.ask("mkdir")}
     HarborButton{text:"Copy";enabled:selected.length>0;onClicked:Browser.copy(selected,false)}
     HarborButton{text:"Cut";enabled:selected.length>0;onClicked:Browser.copy(selected,true)}
     HarborButton{text:"Paste";enabled:!Browser.busy;onClicked:Browser.paste()}
     HarborButton{text:"•••";onClicked:actions.open();Menu{id:actions
      MenuItem{text:"Rename";enabled:selected.length===1&&!Browser.busy;onTriggered:root.ask("rename")}
      MenuItem{text:"Move to Trash";enabled:selected.length>0&&!Browser.busy;onTriggered:trashDialog.open()}
      MenuItem{text:"Refresh";onTriggered:Browser.refresh()}
     }}
     Item{Layout.fillWidth:true}
    }
    Item{id:fileArea;focus:true;Layout.fillWidth:true;Layout.fillHeight:true
     DropArea{anchors.fill:parent;onDropped:drop=>{if(drop.hasUrls){Browser.drop(drop.urls,(drop.modifiers&Qt.ShiftModifier)!==0);drop.acceptProposedAction()}}}
     GridView{id:grid;anchors.fill:parent;visible:root.viewMode===0;clip:true;model:Browser.entries;cellWidth:112;cellHeight:116
      delegate:FileTile{required property var modelData;width:112;height:116;entry:modelData}
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
   Rectangle {Layout.preferredWidth:230;Layout.fillHeight:true;visible:root.previewVisible;color:Prefs.dark?"#17263a":"#edf2f8"
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
  RowLayout{Layout.fillWidth:true;Layout.margins:10
   BusyIndicator{running:Browser.busy;visible:running;Layout.preferredWidth:22;Layout.preferredHeight:22}
   HarborLabel{text:Browser.error||Browser.entries.length+" items · "+selected.length+" selected";elide:Text.ElideRight;Layout.fillWidth:true;font.pixelSize:12}
  }
 }
 component FileTile: Rectangle {
  id:tile;objectName:"fileTile-"+entry.name;property var entry;property bool compact:false
  color:root.selected.includes(entry.path)?(Prefs.dark?"#35536c":"#c5e2ea"):hover.containsMouse?(Prefs.dark?"#22354a":"#e5edf5"):"transparent";radius:8
  Image{x:compact?8:(parent.width-48)/2;y:compact?8:10;width:compact?26:48;height:width;source:"image://icons/"+entry.icon}
  Text{x:compact?42:5;y:compact?0:64;width:compact?(parent.width>400?parent.width-255:parent.width-50):parent.width-10;height:compact?parent.height:46;text:entry.name+(entry.link?" ↗":"");color:Prefs.dark?"#eef4ff":"#17314c";font.pixelSize:12;wrapMode:compact?Text.NoWrap:Text.Wrap;elide:Text.ElideRight;maximumLineCount:2;horizontalAlignment:compact?Text.AlignLeft:Text.AlignHCenter;verticalAlignment:Text.AlignVCenter}
  Text{visible:tile.compact&&tile.width>400;anchors.right:parent.right;anchors.rightMargin:8;anchors.verticalCenter:parent.verticalCenter;text:tile.entry.folder?tile.entry.modified:tile.entry.size.toLocaleString()+" B  ·  "+tile.entry.modified;color:Prefs.dark?"#a6bacd":"#4b637a";font.pixelSize:11}
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
 Dialog{id:nameDialog;anchors.centerIn:parent;modal:true;title:root.pendingAction==="mkdir"?"New folder":"Rename";standardButtons:Dialog.Ok|Dialog.Cancel
  HarborField{id:nameField;width:300;onAccepted:nameDialog.accept()}
  onAccepted:Browser.operate(root.pendingAction,root.selected,nameField.text)
 }
 Dialog{id:trashDialog;anchors.centerIn:parent;modal:true;title:"Move "+root.selected.length+" item(s) to Trash?";standardButtons:Dialog.Ok|Dialog.Cancel
  Label{text:"Files are sent to the system Trash, not permanently deleted."}
  onAccepted:Browser.operate("trash",root.selected)
 }
}
