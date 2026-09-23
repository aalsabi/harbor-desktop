import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
GlassCard {
 id:root;focus:true;Keys.onEscapePressed:UI.dismiss()
 LayoutMirroring.enabled:Prefs.language==="ar";LayoutMirroring.childrenInherit:true
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 component Section:Rectangle {
  default property alias contents:body.data
  Layout.fillWidth:true;implicitHeight:body.implicitHeight+24;radius:14;color:Prefs.dark?"#aa33333a":"#ccffffff";border.color:Prefs.dark?"#44555560":"#33777788"
  ColumnLayout{id:body;anchors.fill:parent;anchors.margins:12;spacing:8}
 }
 component Tile:HarborButton {
  property string glyph;property string label;property string detail;property bool on:false
  Layout.fillWidth:true;implicitHeight:76;text:"";Accessible.name:label
  background:Rectangle{radius:14;color:parent.on?Prefs.accent:Prefs.dark?"#aa33333a":"#ccffffff"}
  contentItem:Column{spacing:4;Text{text:parent.parent.glyph+"  "+parent.parent.label;color:parent.parent.on?"white":Prefs.dark?"#eeeeef":"#262628";font.bold:true;font.pixelSize:14}Text{text:parent.parent.detail;color:parent.parent.on?"#e9f4ff":Prefs.dark?"#b9b9c3":"#72727b";font.pixelSize:11}}
 }
 ColumnLayout{anchors.fill:parent;anchors.margins:16;spacing:12
  RowLayout{HarborLabel{text:root.t("Control Center","مركز التحكم");font.pixelSize:18;font.bold:true;Layout.fillWidth:true}HarborButton{text:"↻";Accessible.name:"Refresh controls";onClicked:System.refresh()}HarborButton{text:"×";Accessible.name:"Close control center";onClicked:UI.dismiss()}}
  ScrollView{Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
   ColumnLayout{width:parent.width;spacing:10
    RowLayout{Layout.fillWidth:true;spacing:10
     Tile{objectName:"control-wifi";glyph:"◔";label:"Wi-Fi";detail:!System.state.wifiAvailable?root.t("Unavailable","غير متاح"):on?root.t("On","تشغيل"):root.t("Off","إيقاف");on:System.state.wifi==="enabled";enabled:!!System.state.wifiAvailable&&!System.busy;onClicked:System.action("wifi",!on)}
     Tile{objectName:"control-bluetooth";glyph:"ᛒ";label:"Bluetooth";detail:!System.state.bluetoothAvailable?root.t("Unavailable","غير متاح"):on?root.t("On","تشغيل"):root.t("Off","إيقاف");on:(System.state.bluetooth||"").includes("Powered: yes");enabled:!!System.state.bluetoothAvailable&&!System.busy;onClicked:System.action("bluetooth",!on)}
    }
    RowLayout{HarborButton{text:root.t("Networks…","الشبكات…");Layout.fillWidth:true;onClicked:UI.open("settings:Wi-Fi")}HarborButton{text:root.t("Devices…","الأجهزة…");Layout.fillWidth:true;onClicked:UI.open("settings:Bluetooth")}}
    Section{RowLayout{HarborLabel{text:root.t("Display","الشاشة");font.bold:true;Layout.fillWidth:true}HarborLabel{text:System.state.brightnessAvailable?Math.round(System.state.brightnessPercent||0)+"%":root.t("Unavailable","غير متاح")}}
     Slider{objectName:"control-brightness";Layout.fillWidth:true;from:5;to:100;value:System.state.brightnessPercent||0;enabled:!!System.state.brightnessAvailable;onMoved:{brightnessTimer.requestedValue=value;brightnessTimer.restart()}Timer{id:brightnessTimer;property real requestedValue:0;interval:180;onTriggered:System.action("brightness",Math.round(requestedValue))}}
     RowLayout{HarborButton{text:Prefs.dark?root.t("Light","فاتح"):root.t("Dark","داكن");onClicked:Prefs.dark=!Prefs.dark}HarborButton{text:root.t("Displays…","الشاشات…");onClicked:UI.open("settings:Displays")}Item{Layout.fillWidth:true}}
    }
    Section{RowLayout{HarborLabel{text:root.t("Sound","الصوت");font.bold:true;Layout.fillWidth:true}HarborLabel{text:System.state.volumeAvailable?Math.round((System.state.outputVolume||0)*100)+"%":root.t("Unavailable","غير متاح")}}
     Slider{objectName:"control-volume";Layout.fillWidth:true;from:0;to:1;value:System.state.outputVolume||0;enabled:!!System.state.volumeAvailable;onMoved:{volumeTimer.requestedValue=value;volumeTimer.restart()}Timer{id:volumeTimer;property real requestedValue:0;interval:180;onTriggered:System.action("volume",requestedValue)}}
     RowLayout{HarborButton{text:System.state.outputMuted?root.t("Unmute","إلغاء الكتم"):root.t("Mute","كتم");enabled:!!System.state.volumeAvailable&&!System.busy;onClicked:System.action("mute")}HarborButton{text:root.t("Output & Input…","الإخراج والإدخال…");onClicked:UI.open("settings:Sound")}}
    }
    Section{RowLayout{HarborLabel{text:root.t("Keyboard","لوحة المفاتيح");font.bold:true;Layout.fillWidth:true}HarborButton{text:root.t("Switch","تبديل");enabled:!!System.state.harborSession;onClicked:Keyboard.switchNext()}HarborButton{text:"…";Accessible.name:"Keyboard settings";onClicked:UI.open("settings:Keyboard")}}
     HarborLabel{text:Keyboard.message;visible:text.length>0;wrapMode:Text.WordWrap;Layout.fillWidth:true;font.pixelSize:11}
    }
    Section{RowLayout{HarborLabel{text:root.t("Power","الطاقة");font.bold:true;Layout.fillWidth:true}HarborLabel{text:System.state.batteryAvailable?Math.round(System.state.batteryPercent)+"%"+(System.state.batteryCharging?" ⚡":""):""}}
     ComboBox{Layout.fillWidth:true;model:System.state.powerProfiles||[];currentIndex:(System.state.powerProfiles||[]).indexOf(System.state.power||"");enabled:!!System.state.powerAvailable&&!System.busy;onActivated:if(currentIndex>=0)System.action("power",currentText)}
     HarborButton{text:root.t("Battery settings…","إعدادات الطاقة…");onClicked:UI.open("settings:Battery")}
    }
    HarborButton{text:root.t("Lock Screen","قفل الشاشة");enabled:!!System.state.lockAvailable;Layout.fillWidth:true;onClicked:System.action("lock");ToolTip.visible:hovered&&!enabled;ToolTip.text:root.t("No lock service is running","خدمة القفل غير متاحة")}
    SessionActions{Layout.fillWidth:true}
    HarborLabel{text:System.message;visible:text.length>0;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:Prefs.dark?"#edbd80":"#86571e";font.pixelSize:12}
   }
  }
  HarborButton{text:root.t("System Settings…","إعدادات النظام…");Layout.fillWidth:true;prominent:true;onClicked:UI.open("settings")}
 }
}
