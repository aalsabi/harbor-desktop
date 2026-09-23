import QtQuick
import QtQuick.Controls
Item {
 GlassCard {anchors.centerIn:parent;height:parent.height-8;width:Math.min(parent.width-8,row.width+24);radius:23}
 Flickable {id:flick;anchors.fill:parent;anchors.margins:10;contentWidth:Math.max(width,row.width);clip:true;flickableDirection:Flickable.HorizontalFlick
  Row {id:row;x:Math.max(0,(flick.width-width)/2);height:58;spacing:9
   Repeater {model:[{icon:"view-app-grid",name:"Applications",action:"launcher"},{icon:"system-file-manager",name:"Files",action:"files"},{icon:"utilities-terminal",name:"Terminal",action:"terminal"},{icon:"preferences-system",name:"Settings",action:"settings"}]
    delegate:HarborButton {required property var modelData;property string desktopId:Apps.desktopForTool(modelData.action);property bool launching:Apps.pendingLaunches.includes(desktopId);enabled:!launching;width:Prefs.dockIconSize+10;height:58;background:Rectangle{radius:12;color:parent.hovered?"#44ffffff":"transparent"}
text:"";Accessible.name:modelData.name;Image{anchors.centerIn:parent;width:Prefs.dockIconSize;height:Prefs.dockIconSize;source:"image://icons/"+encodeURIComponent(modelData.icon)}onClicked:{if(desktopId)Apps.launch(desktopId);else if(modelData.action==="files"||modelData.action==="terminal")System.openTool(modelData.action);else UI.open(modelData.action)}BusyIndicator{anchors.centerIn:parent;width:24;height:24;running:parent.launching;visible:running}}
   }
   Rectangle{width:1;height:38;anchors.verticalCenter:parent.verticalCenter;color:"#446e9bac"}
   Repeater {model:Prefs.pins.filter(id=>Apps.entries.some(a=>a.id===id))
    delegate:HarborButton {required property string modelData;objectName:"dock-app-"+modelData;property var entry:Apps.entries.find(a=>a.id===modelData)||({name:modelData,icon:"app"});property bool launching:Apps.pendingLaunches.includes(modelData);enabled:!launching;width:Prefs.dockIconSize+10;height:58;background:Rectangle{radius:12;color:parent.hovered?"#44ffffff":"transparent"}
text:""
     Image{anchors.centerIn:parent;width:Prefs.dockIconSize;height:Prefs.dockIconSize;source:"image://icons/"+encodeURIComponent(parent.entry.icon)}
     BusyIndicator{anchors.centerIn:parent;width:24;height:24;running:parent.launching;visible:running}
     onClicked:Apps.launch(modelData)
     onPressAndHold:Prefs.pins=Prefs.pins.filter(id=>id!==modelData)
     Accessible.name:entry.name
     Rectangle{width:4;height:4;radius:2;anchors.bottom:parent.bottom;anchors.horizontalCenter:parent.horizontalCenter;color:"#81ddd0";visible:Windows.windows.some(w=>w.appId===modelData||w.appId+".desktop"===modelData)}
    }
   }
   Repeater {model:Windows.windows
    delegate:HarborButton{required property var modelData;width:Prefs.dockIconSize+10;height:58;background:Rectangle{radius:12;color:parent.hovered?"#44ffffff":"transparent"}
text:"";Image{anchors.centerIn:parent;width:Prefs.dockIconSize;height:Prefs.dockIconSize;source:"image://icons/"+encodeURIComponent(Apps.iconForAppId(modelData.appId))}
Accessible.name:modelData.title;prominent:modelData.active;onClicked:Windows.activate(modelData.id);onPressAndHold:UI.open("windows");}
   }
  }
 }
}
