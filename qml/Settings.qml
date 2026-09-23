import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 LayoutMirroring.enabled: Prefs.language==="ar"
 LayoutMirroring.childrenInherit:true
 radius:18
 id:root;color:Prefs.dark?"#29292d":"#f7f7f9"
 property string section: "Appearance"
 RowLayout {anchors.fill:parent;spacing:0
  Rectangle {Layout.preferredWidth:222;Layout.fillHeight:true;color:Prefs.dark?"#333338":"#ececf1"
   ColumnLayout {anchors.fill:parent;anchors.margins:20;spacing:8
    Row {spacing:8;Repeater{model:["#fa7773","#f3c576","#80d6b8"];delegate:Rectangle{required property string modelData;required property int index;width:12;height:12;radius:6;color:modelData;MouseArea{anchors.fill:parent;onClicked:UI.windowAction(index===0?"close":index===1?"minimize":"maximize")}}}}
    HarborLabel{text:"Harbor";font.pixelSize:20;font.bold:true;Layout.topMargin:12;MouseArea{anchors.fill:parent;onPressed:UI.windowAction("move")}}
    HarborLabel{text:qsTr("System settings");opacity:.6;Layout.bottomMargin:16}
    HarborField{id:filter;Layout.fillWidth:true;placeholderText:qsTr("Find a setting")}
    Repeater {model:["Appearance","Network","Sound","Bluetooth","Displays","Power","Users","Updates","About"].filter(x=>x.toLowerCase().includes(filter.text.toLowerCase()))
     delegate:HarborButton{required property string modelData;text:qsTr(modelData);Layout.fillWidth:true;prominent:root.section===modelData;onClicked:root.section=modelData}
    }
    Item{Layout.fillHeight:true}
    HarborLabel{text:"KWin · Wayland\nIndependent by design";opacity:.55;font.pixelSize:11}
   }
  }
  ScrollView {Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
   ColumnLayout {width:parent.width;spacing:18
    Item{height:14}
    HarborLabel {text:qsTr(root.section);font.pixelSize:23;font.bold:true;Layout.leftMargin:30}
    HarborLabel {text:qsTr("Make this desktop your own.");opacity:.6;Layout.leftMargin:30;visible:root.section==="Appearance"}
    ColumnLayout {visible:root.section==="Appearance";Layout.fillWidth:true;Layout.margins:30;spacing:20
     RowLayout {HarborButton{text:qsTr("Dark");prominent:Prefs.dark;onClicked:Prefs.dark=true;Layout.preferredWidth:150}
HarborButton{text:qsTr("Light");prominent:!Prefs.dark;onClicked:Prefs.dark=false;Layout.preferredWidth:150}}
     Image{source:"qrc:/assets/wallpapers/harbor.svg";Layout.fillWidth:true;Layout.preferredHeight:150;fillMode:Image.PreserveAspectCrop}
     HarborLabel{text:qsTr("Glass opacity");font.bold:true}
     Slider{Layout.fillWidth:true;from:.45;to:1;value:Prefs.opacity;onMoved:Prefs.opacity=value}
     HarborLabel{text:qsTr("Adjust the transparency of desktop panels.");wrapMode:Text.Wrap;Layout.fillWidth:true;opacity:.65}
     HarborButton{text:Prefs.language==="ar"?"English":"العربية";onClicked:Prefs.language=Prefs.language==="ar"?"en":"ar"}
     Switch{palette.windowText:Prefs.dark?"#edf3fa":"#182e47";text:qsTr("Reduce motion");checked:Prefs.reduceMotion;onToggled:Prefs.reduceMotion=checked}
    }
    ColumnLayout {visible:root.section==="Network";Layout.fillWidth:true;Layout.margins:30
     HarborLabel{text:System.state.network||qsTr("NetworkManager is unavailable");wrapMode:Text.Wrap;Layout.fillWidth:true}
     Switch{text:"Wi-Fi";checked:System.state.wifi==="enabled";enabled:System.state.wifiAvailable&&!System.busy;onToggled:System.action("wifi",checked)}
     HarborLabel{text:qsTr("Nearby networks");font.bold:true}
     HarborLabel{text:System.state.networks||qsTr("No scan results");wrapMode:Text.Wrap;Layout.fillWidth:true}
     HarborButton{text:qsTr("Manage connections — external editor");onClicked:System.openTool("network")}
    }
    ColumnLayout {visible:root.section==="Sound";Layout.fillWidth:true;Layout.margins:30
     HarborLabel{text:System.state.volume||qsTr("Audio service unavailable")}
     Slider{from:0;to:1;value:parseFloat((System.state.volume||"Volume: 0").split(" ")[1])||0;enabled:System.state.volumeAvailable&&!System.busy;Layout.fillWidth:true;onMoved:audioTimer.restart();Timer{id:audioTimer;interval:180;onTriggered:System.action("volume",parent.value)}}
     HarborButton{text:qsTr("Mute / unmute");onClicked:System.action("mute");enabled:System.state.volumeAvailable&&!System.busy}
     HarborButton{text:qsTr("Audio devices — external mixer");onClicked:System.openTool("audio")}
     HarborLabel{text:System.state.audio||"";font.family:"monospace";font.pixelSize:11;wrapMode:Text.Wrap;Layout.fillWidth:true}
    }
    ColumnLayout {visible:root.section==="Bluetooth";Layout.fillWidth:true;Layout.margins:30
     HarborLabel{text:System.state.bluetooth||qsTr("Bluetooth service unavailable");wrapMode:Text.Wrap;Layout.fillWidth:true}
     HarborButton{text:qsTr("Pair devices — external manager");onClicked:System.openTool("bluetooth")}
     HarborLabel{text:System.state.devices||"";wrapMode:Text.Wrap;Layout.fillWidth:true}
    }
    ColumnLayout {visible:root.section==="Displays";Layout.fillWidth:true;Layout.margins:30
     HarborLabel{text:qsTr("A display change reverts after 15 seconds unless confirmed.");wrapMode:Text.Wrap;Layout.fillWidth:true}
     Repeater{model:System.state.displayOutputs||[]
      delegate:ColumnLayout{required property var modelData;Layout.fillWidth:true;visible:modelData.connected
       HarborLabel{text:modelData.name;font.bold:true}
       RowLayout{
        ComboBox{id:scale;model:["100%","125%","150%","175%","200%"];currentIndex:Math.max(0,Math.round((modelData.scale-1)*4))}
        ComboBox{id:mode;model:modelData.modes||[];currentIndex:(modelData.modes||[]).findIndex(m=>String(m.id)===String(modelData.currentModeId));textRole:"name";Layout.fillWidth:true}
        HarborButton{text:qsTr("Apply");enabled:!System.state.displayChanging;onClicked:System.applyDisplay(modelData.id,1+scale.currentIndex*.25,mode.currentIndex>=0?String(modelData.modes[mode.currentIndex].id):"")}
       }
      }
     }
     HarborButton{text:qsTr("Keep display settings");visible:!!System.state.displayPending;prominent:true;onClicked:System.confirmDisplay()}
    }
    ColumnLayout {visible:root.section==="Power";Layout.fillWidth:true;Layout.margins:30
     HarborLabel{text:qsTr("Current profile: ")+(System.state.power||qsTr("Unavailable"))}
     Repeater{model:["power-saver","balanced","performance"];delegate:HarborButton{required property string modelData;text:modelData;enabled:System.state.powerAvailable&&!System.busy;onClicked:System.action("power",modelData)}}
    }
    ColumnLayout {visible:root.section==="Users";Layout.fillWidth:true;Layout.margins:30;spacing:16
     HarborLabel{text:qsTr("Local accounts");font.bold:true}
     HarborLabel{text:Accounts.error;visible:text.length>0;wrapMode:Text.Wrap;Layout.fillWidth:true;color:"#dba15d"}
     Repeater{model:Accounts.users;delegate:ColumnLayout{required property var modelData;Layout.fillWidth:true
      HarborLabel{text:modelData.UserName+" · "+(modelData.AccountType===1?"Administrator":"Standard")}
      RowLayout{HarborField{id:realName;text:modelData.RealName;Layout.fillWidth:true} HarborButton{text:qsTr("Apply");enabled:!Accounts.busy;onClicked:Accounts.setRealName(modelData.path,realName.text)}}
     }}
     HarborLabel{text:qsTr("Name changes use AccountsService and system authentication. Creating users and changing passwords are not implemented.");wrapMode:Text.Wrap;Layout.fillWidth:true;opacity:.65}
    }
    ColumnLayout {visible:root.section==="Updates";Layout.fillWidth:true;Layout.margins:30
     HarborLabel{text:qsTr("Administrative changes use an external system tool and its authentication policy.");wrapMode:Text.Wrap;Layout.fillWidth:true}
     HarborButton{text:root.section==="Users"?qsTr("Open user manager"):qsTr("Open software manager");onClicked:System.openTool(root.section==="Users"?"users":"updates")}
    }
    ColumnLayout {visible:root.section==="About";Layout.fillWidth:true;Layout.margins:30;spacing:16
     HarborLabel{text:"Harbor Desktop 0.1.0";font.pixelSize:24}
     HarborLabel{text:qsTr("An independent desktop shell built with Qt and KDE technologies.\n\nKWin manages windows. Harbor provides the desktop interface. Plasma Shell is not used.\n\nOriginal artwork · GPL-3.0-or-later\nExperimental build: see the compatibility report before daily use.");wrapMode:Text.Wrap;Layout.fillWidth:true}
    }
    HarborLabel{text:System.message;visible:text.length>0;color:"#d69a53";wrapMode:Text.Wrap;Layout.fillWidth:true;Layout.margins:30}
   }
  }
 }
 MouseArea{anchors.right:parent.right;anchors.bottom:parent.bottom;width:18;height:18;cursorShape:Qt.SizeFDiagCursor;onPressed:UI.windowAction("resize")}
}
