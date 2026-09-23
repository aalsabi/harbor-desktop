import QtQuick
import QtQuick.Controls
Item {
 GlassCard {anchors.fill:parent;anchors.margins:4;radius:23}
 Flickable {id:flick;anchors.fill:parent;anchors.margins:10;contentWidth:Math.max(width,row.width);clip:true;flickableDirection:Flickable.HorizontalFlick
  Row {id:row;x:Math.max(0,(flick.width-width)/2);height:58;spacing:9
   Repeater {model:[{icon:"view-app-grid",name:"Applications",action:"launcher"},{icon:"system-file-manager",name:"Files",action:"files"},{icon:"utilities-terminal",name:"Terminal",action:"terminal"},{icon:"preferences-system",name:"Settings",action:"settings"}]
    delegate:HarborButton {required property var modelData;width:54;height:58;text:"";Accessible.name:modelData.name;Image{anchors.centerIn:parent;width:40;height:40;source:"image://icons/"+modelData.icon}onClicked:modelData.action==="files"||modelData.action==="terminal"?System.openTool(modelData.action):UI.open(modelData.action);ToolTip.visible:hovered;ToolTip.text:modelData.name}
   }
   Rectangle{width:1;height:38;anchors.verticalCenter:parent.verticalCenter;color:"#446e9bac"}
   Repeater {model:Prefs.pins.filter(id=>Apps.entries.some(a=>a.id===id))
    delegate:HarborButton {required property string modelData;property var entry:Apps.entries.find(a=>a.id===modelData)||({name:modelData,icon:"app"});width:54;height:58;text:""
     Image{anchors.centerIn:parent;width:36;height:36;source:"image://icons/"+parent.entry.icon}
     onClicked:{let w=Windows.windows.find(w=>w.appId===modelData||w.appId+".desktop"===modelData);if(w)Windows.activate(w.id);else Apps.launch(modelData)}
     onPressAndHold:Prefs.pins=Prefs.pins.filter(id=>id!==modelData)
     ToolTip.visible:hovered;ToolTip.text:entry.name;Accessible.name:entry.name
     Rectangle{width:4;height:4;radius:2;anchors.bottom:parent.bottom;anchors.horizontalCenter:parent.horizontalCenter;color:"#81ddd0";visible:Windows.windows.some(w=>w.appId===modelData||w.appId+".desktop"===modelData)}
    }
   }
   Repeater {model:Windows.windows
    delegate:HarborButton{required property var modelData;width:54;height:58;text:modelData.title.substring(0,2);prominent:modelData.active;onClicked:Windows.activate(modelData.id);onPressAndHold:UI.open("windows");ToolTip.visible:hovered;ToolTip.text:modelData.title}
   }
  }
 }
}
