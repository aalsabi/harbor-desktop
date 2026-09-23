import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id:root;radius:16;clip:true;color:Prefs.dark?"#242426":"#f5f5f7"
 LayoutMirroring.enabled:Prefs.language==="ar";LayoutMirroring.childrenInherit:true
 property real previousOpacity:Prefs.opacity<1?Prefs.opacity:.9
 property string section:"General"
 property color ink:Prefs.dark?"#eeeef0":"#252527"
 property color muted:Prefs.dark?"#a6a6ad":"#76767c"
 property color card:Prefs.dark?"#303034":"#ffffff"
 property color line:Prefs.dark?"#454549":"#e2e2e7"
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 property var pages:[
  {key:"Wi-Fi",ar:"واي فاي",icon:"◔",color:Prefs.accent,id:"wifi",group:0,tags:"wireless internet شبكة"},
  {key:"Bluetooth",ar:"بلوتوث",icon:"ᛒ",color:Prefs.accent,id:"bluetooth",group:0,tags:"devices أجهزة"},
  {key:"Network",ar:"الشبكة",icon:"◎",color:"#2488d8",id:"network",group:0,tags:"internet ethernet"},
  {key:"Sound",ar:"الصوت",icon:"♪",color:"#ed4b66",id:"sound",group:1,tags:"volume audio صوت"},
  {key:"General",ar:"عام",icon:"⚙",color:"#8a8b90",id:"general",group:2,tags:"about updates language تحديث لغة"},
  {key:"Appearance",ar:"المظهر",icon:"◐",color:"#77777e",id:"appearance",group:2,tags:"light dark theme مظهر"},
  {key:"Accessibility",ar:"تسهيلات الاستخدام",icon:"◎",color:"#258de9",id:"accessibility",group:2,tags:"motion movement حركة"},
  {key:"Desktop & Dock",ar:"سطح المكتب وDock",icon:"▣",color:"#437ee9",id:"desktop",group:3,tags:"panel glass transparency شفافية"},
  {key:"Displays",ar:"الشاشات",icon:"▱",color:"#7274e5",id:"displays",group:3,tags:"resolution scale brightness دقة سطوع"},
  {key:"Keyboard",ar:"لوحة المفاتيح",icon:"⌨",color:"#85858d",id:"keyboard",group:3,tags:"input language arabic english shortcut كيبورد عربي انجليزي لغة"},
  {key:"Battery",ar:"الطاقة",icon:"▰",color:"#39a958",id:"battery",group:3,tags:"power performance energy طاقة"},
  {key:"Users & Groups",ar:"المستخدمون والمجموعات",icon:"♙",color:"#5a83ce",id:"users",group:4,tags:"account name حساب"}
 ]
 property var results:pages.filter(p=>(p.key+" "+p.ar+" "+p.tags).toLowerCase().includes(search.text.toLowerCase()))
 property var currentAccount:Accounts.users.find(u=>u.UserName===System.state.userName)||null
 property var currentPage:pages.find(p=>p.key===section)||({key:section,ar:section==="About"?"حول":"تحديث البرامج",icon:"⚙",color:"#8a8b90"})
 onSectionChanged:pageScroll.contentItem.contentY=0
 component Label:Text {color:root.ink;font.pixelSize:13;wrapMode:Text.WordWrap;Layout.fillWidth:true}
 component Note:Label {color:root.muted;font.pixelSize:12;lineHeight:1.2}
 component Group:Rectangle {
  default property alias contents:groupBody.data
  Layout.fillWidth:true;implicitHeight:groupBody.implicitHeight+28;radius:10;color:root.card;border.color:root.line
  ColumnLayout{id:groupBody;anchors.fill:parent;anchors.margins:14;spacing:12}
 }
 component Divider:Rectangle {Layout.fillWidth:true;height:1;color:root.line}
 component SettingRow:RowLayout {
  property string label;property string hint:""
  default property alias controls:controlRow.data
  Layout.fillWidth:true;spacing:18
  ColumnLayout{Layout.fillWidth:true;spacing:4;Label{text:parent.parent.label}Note{text:parent.parent.hint;visible:text.length>0}}
  RowLayout{id:controlRow;spacing:8}
 }
 component LinkRow:Button {
  property string destination
  Layout.fillWidth:true;implicitHeight:39;text:destination;onClicked:root.section=destination
  background:Rectangle{radius:6;color:parent.hovered?(Prefs.dark?"#45454b":"#f0f0f5"):"transparent"}
  contentItem:RowLayout{Label{text:parent.parent.text}Text{text:Prefs.language==="ar"?"‹":"›";font.pixelSize:23;color:root.muted}}
 }
 component Select:ComboBox {
  id:select;implicitHeight:30;implicitWidth:170;font.pixelSize:13;leftPadding:10;rightPadding:27
  contentItem:Text{text:select.displayText;color:root.ink;font:select.font;verticalAlignment:Text.AlignVCenter;elide:Text.ElideRight}
  background:Rectangle{radius:6;color:Prefs.dark?"#45454b":"#fafafa";border.color:select.activeFocus?Prefs.accent:root.line;border.width:select.activeFocus?2:1}
  indicator:Text{x:select.width-width-9;y:(select.height-height)/2;text:"⌄";color:root.muted;font.pixelSize:16}
 }
 component ThemeChoice:Button {
  property bool night:false
  implicitWidth:154;implicitHeight:105;onClicked:Prefs.dark=night;Accessible.name:night?"Dark appearance":"Light appearance"
  background:Rectangle{radius:9;color:parent.night?"#22222b":"#e0eaff";border.width:Prefs.dark===parent.night?3:1;border.color:Prefs.dark===parent.night?Prefs.accent:root.line
   Rectangle{anchors.centerIn:parent;width:112;height:72;radius:6;color:parent.parent.night?"#36363d":"#ffffff";Rectangle{width:30;height:parent.height;radius:6;color:parent.parent.parent.night?"#4a4a52":"#e8e8ef"}Column{anchors.centerIn:parent;anchors.horizontalCenterOffset:12;spacing:7;Repeater{model:3;Rectangle{width:54;height:5;radius:2;color:night?"#60606d":"#d9d9e2"}}}}
  }
  contentItem:Item{}
 }
 RowLayout{anchors.fill:parent;spacing:0
  Rectangle{Layout.preferredWidth:238;Layout.fillHeight:true;color:Prefs.dark?"#2e2e33":"#e9e9ef"
   ColumnLayout{anchors.fill:parent;anchors.margins:12;spacing:10
    Item{Layout.fillWidth:true;Layout.preferredHeight:30
     MouseArea{anchors.fill:parent;onPressed:UI.windowAction("move")}
     Row{anchors.verticalCenter:parent.verticalCenter;spacing:0;Repeater{model:["#ff6057","#febc2e","#28c840"];delegate:Button{required property string modelData;required property int index;width:24;height:26;Accessible.name:["Close window","Minimize window","Maximize window"][index];contentItem:Item{} background:Item{Rectangle{anchors.centerIn:parent;width:12;height:12;radius:6;color:parent.parent.modelData;border.width:parent.parent.activeFocus?2:1;border.color:parent.parent.activeFocus?Prefs.accent:Qt.darker(color,1.12)}}onClicked:UI.windowAction(index===0?"close":index===1?"minimize":"maximize")}}}
    }
    HarborField{id:search;objectName:"settings-search";Layout.fillWidth:true;implicitHeight:30;placeholderText:root.t("Search","بحث");font.pixelSize:13}
    Button{Layout.fillWidth:true;implicitHeight:58;onClicked:root.section="Users & Groups";background:Rectangle{color:parent.hovered?(Prefs.dark?"#414148":"#dedee6"):"transparent";radius:8}
     contentItem:RowLayout{spacing:10;Rectangle{width:40;height:40;radius:20;color:"#a0a6b5";Text{anchors.centerIn:parent;text:"♙";font.pixelSize:28;color:"white"}}ColumnLayout{spacing:2;Label{text:root.currentAccount?(root.currentAccount.RealName||root.currentAccount.UserName):root.t("Local account","الحساب المحلي");font.bold:true;elide:Text.ElideRight;wrapMode:Text.NoWrap}Note{text:root.t("Account settings","إعدادات الحساب")}}}
    }
    ScrollView{Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
     ColumnLayout{width:parent.width;spacing:3
      Repeater{model:root.results;delegate:Button{required property var modelData;required property int index
       objectName:"settings-nav-"+modelData.id;Layout.fillWidth:true;Layout.preferredHeight:33;Layout.topMargin:index>0&&root.results[index-1].group!==modelData.group?9:0
       Accessible.name:root.t(modelData.key,modelData.ar);onClicked:root.section=modelData.key
       background:Rectangle{radius:6;color:root.section===parent.modelData.key?Prefs.accent:parent.hovered?(Prefs.dark?"#414148":"#dcdce5"):"transparent"}
       contentItem:RowLayout{spacing:9;Rectangle{width:25;height:25;radius:6;color:modelData.color;Text{anchors.centerIn:parent;text:modelData.icon;color:"white";font.pixelSize:19}}Text{text:root.t(modelData.key,modelData.ar);color:root.section===modelData.key?"white":root.ink;font.pixelSize:13;Layout.fillWidth:true;elide:Text.ElideRight}}
      }}
      Note{visible:root.results.length===0;text:root.t("No matching settings","لا توجد نتائج")}
     }
    }
   }
  }
  Rectangle{Layout.preferredWidth:1;Layout.fillHeight:true;color:root.line}
  ColumnLayout{Layout.fillWidth:true;Layout.fillHeight:true;spacing:0
   Item{Layout.fillWidth:true;Layout.preferredHeight:60
    MouseArea{anchors.fill:parent;onPressed:UI.windowAction("move");onDoubleClicked:UI.windowAction("maximize")}
    RowLayout{anchors.fill:parent;anchors.leftMargin:24;anchors.rightMargin:24
     HarborButton{text:"‹";visible:root.section==="About"||root.section==="Software Update";onClicked:root.section="General";Accessible.name:"Back to General"}
     Label{text:root.t(root.currentPage.key,root.currentPage.ar);font.bold:true;font.pixelSize:20}
    }
   }
   ScrollView{id:pageScroll;Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
    ColumnLayout{width:pageScroll.availableWidth;spacing:18
     ColumnLayout{Layout.fillWidth:true;Layout.leftMargin:24;Layout.rightMargin:24;Layout.bottomMargin:28;spacing:18
      ColumnLayout{visible:root.section==="General";Layout.fillWidth:true;spacing:18
       Rectangle{Layout.alignment:Qt.AlignHCenter;width:64;height:64;radius:15;color:"#888a93";Text{anchors.centerIn:parent;text:"⚙";font.pixelSize:49;color:"white"}}
       Label{text:root.t("General","عام");font.pixelSize:24;font.bold:true;horizontalAlignment:Text.AlignHCenter}
       Note{text:root.t("Manage your desktop, language and system information.","إدارة سطح المكتب واللغة ومعلومات النظام.");horizontalAlignment:Text.AlignHCenter}
       Group{LinkRow{destination:"About";text:root.t("About","حول")}Divider{}LinkRow{destination:"Software Update";text:root.t("Software Update","تحديث البرامج")}}
       Group{SettingRow{label:root.t("Interface language","لغة الواجهة");hint:root.t("Harbor interface language","لغة واجهة Harbor");Select{model:["English","العربية"];currentIndex:Prefs.language==="ar"?1:0;onActivated:Prefs.language=currentIndex===1?"ar":"en"}}
        Divider{}LinkRow{destination:"Keyboard";text:root.t("Keyboard input sources","لغات الكتابة")}}
       Group{Label{text:root.t("Session","الجلسة");font.bold:true}SessionActions{}Note{text:root.t("Save your work before signing out or powering off.","احفظ عملك قبل تسجيل الخروج أو إيقاف التشغيل.")}}
      }
      ColumnLayout{visible:root.section==="Appearance";Layout.fillWidth:true;spacing:18
       Group{Label{text:root.t("Appearance","المظهر");font.bold:true}RowLayout{Layout.alignment:Qt.AlignHCenter;spacing:22
        ColumnLayout{ThemeChoice{night:false;objectName:"appearance-light"}Label{text:root.t("Light","فاتح");horizontalAlignment:Text.AlignHCenter}}
        ColumnLayout{ThemeChoice{night:true;objectName:"appearance-dark"}Label{text:root.t("Dark","داكن");horizontalAlignment:Text.AlignHCenter}}
       }}
       Group{SettingRow{label:root.t("Accent colour","لون التمييز");Row{spacing:8;Repeater{model:["#1684f8","#168044","#7955c9","#b65b00","#c63f75"];delegate:Button{required property string modelData;width:28;height:28;Accessible.name:modelData;contentItem:Item{} background:Rectangle{radius:14;color:parent.modelData;border.width:Prefs.accent===parent.modelData?3:0;border.color:root.ink}onClicked:Prefs.accent=modelData}}}}}
       Note{text:root.t("Appearance applies to Harbor windows and desktop panels. Other apps use their own themes.","يطبّق المظهر على نوافذ Harbor والبانل. للتطبيقات الأخرى ثيماتها الخاصة.")}
      }
      ColumnLayout{visible:root.section==="Wi-Fi"||root.section==="Network";Layout.fillWidth:true;spacing:18
       Group{SettingRow{label:"Wi-Fi";hint:root.t("Wireless networking","الشبكات اللاسلكية");Switch{checked:System.state.wifi==="enabled";enabled:!!System.state.wifiAvailable&&!System.busy;onToggled:System.action("wifi",checked)}}}
       Group{Label{text:root.t("Saved connections","الاتصالات المحفوظة");font.bold:true}
        Repeater{model:System.state.savedConnections||[];delegate:SettingRow{required property var modelData;label:modelData.name;hint:modelData.active?root.t("Connected","متصل"):root.t("Not connected","غير متصل");HarborButton{text:modelData.active?root.t("Disconnect","قطع الاتصال"):root.t("Connect","اتصال");enabled:!System.busy;onClicked:System.action(modelData.active?"connection-down":"connection-up",modelData.uuid)}}}
        Note{visible:!(System.state.savedConnections||[]).length;text:root.t("No saved connections reported.","لا توجد اتصالات محفوظة.")}
       }
       Group{visible:root.section==="Wi-Fi";Label{text:root.t("Nearby networks","الشبكات القريبة");font.bold:true}
        Repeater{model:System.state.wifiNetworks||[];delegate:SettingRow{required property var modelData;label:modelData.name;hint:modelData.active?root.t("Connected","متصل"):(modelData.security==="--"?root.t("Open network","شبكة مفتوحة"):modelData.security);Label{text:modelData.signal+"%";Layout.preferredWidth:55}}}
        Note{visible:!(System.state.wifiNetworks||[]).length;text:root.t("No networks available","لا توجد شبكات متاحة")}
       }
       Group{SettingRow{label:root.t("Connection settings","إعدادات الاتصال");hint:root.t("Add networks, enter passwords or configure advanced options in the connection editor.","إضافة الشبكات وكلمات المرور والخيارات المتقدمة في محرر الاتصالات.");HarborButton{text:root.t("Details…","التفاصيل…");onClicked:System.openTool("network")}}}
      }
      ColumnLayout{visible:root.section==="Bluetooth";Layout.fillWidth:true;spacing:18
       Group{SettingRow{label:"Bluetooth";Switch{checked:(System.state.bluetooth||"").includes("Powered: yes");enabled:!!System.state.bluetoothAvailable&&!System.busy;onToggled:System.action("bluetooth",checked)}}}
       Group{Label{text:root.t("Paired devices","الأجهزة المقترنة");font.bold:true}
        Repeater{model:System.state.bluetoothDevices||[];delegate:SettingRow{required property var modelData;property bool connected:(System.state.bluetoothConnected||[]).includes(modelData.address);label:modelData.name;hint:connected?root.t("Connected","متصل"):root.t("Not connected","غير متصل");HarborButton{text:connected?root.t("Disconnect","قطع الاتصال"):root.t("Connect","اتصال");enabled:!System.busy&&(System.state.bluetooth||"").includes("Powered: yes");onClicked:System.action(connected?"bluetooth-disconnect":"bluetooth-connect",modelData.address)}}}
        Note{visible:!(System.state.bluetoothDevices||[]).length;text:root.t("No paired devices reported","لا توجد أجهزة مقترنة")}
       }
       Group{SettingRow{label:root.t("Connect a device","توصيل جهاز");hint:root.t("Pairing opens the Bluetooth manager","الاقتران يفتح مدير Bluetooth");HarborButton{text:root.t("Pair…","اقتران…");onClicked:System.openTool("bluetooth")}}}
      }
      ColumnLayout{visible:root.section==="Sound";Layout.fillWidth:true;spacing:18
       Group{Label{text:root.t("Output","الإخراج");font.bold:true}SettingRow{label:root.t("Output volume","مستوى الصوت");Label{Layout.preferredWidth:90;text:Math.round((parseFloat((System.state.volume||"Volume: 0").split(" ")[1])||0)*100)+"%"}}
        Slider{Layout.fillWidth:true;from:0;to:1;value:parseFloat((System.state.volume||"Volume: 0").split(" ")[1])||0;enabled:!!System.state.volumeAvailable;onMoved:{audioTimer.requestedValue=value;audioTimer.restart()}Timer{id:audioTimer;property real requestedValue:0;interval:180;onTriggered:System.action("volume",requestedValue)}}
        SettingRow{label:root.t("Mute","كتم الصوت");Switch{checked:(System.state.volume||"").includes("MUTED");enabled:!!System.state.volumeAvailable&&!System.busy;onToggled:System.action("mute")}}}
       Group{SettingRow{label:root.t("Output device","جهاز الإخراج");Select{objectName:"audio-output";Layout.preferredWidth:260;model:System.state.audioOutputs||[];textRole:"name";currentIndex:(System.state.audioOutputs||[]).findIndex(x=>x.id===System.state.defaultOutputId);enabled:count>0&&!System.busy;onActivated:if(currentIndex>=0)System.action("audio-output",model[currentIndex].id)}}
        Divider{}SettingRow{label:root.t("Input device","جهاز الإدخال");Select{objectName:"audio-input";Layout.preferredWidth:260;model:System.state.audioInputs||[];textRole:"name";currentIndex:(System.state.audioInputs||[]).findIndex(x=>x.id===System.state.defaultInputId);enabled:count>0&&!System.busy;onActivated:if(currentIndex>=0)System.action("audio-input",model[currentIndex].id)}}
        Label{text:root.t("Input volume","مستوى صوت الميكروفون");font.bold:true}
        Slider{Layout.fillWidth:true;from:0;to:1;value:Number(System.state.inputVolume)||0;enabled:!!System.state.inputVolumeAvailable;onMoved:{inputTimer.requestedValue=value;inputTimer.restart()}Timer{id:inputTimer;property real requestedValue:0;interval:180;onTriggered:System.action("input-volume",requestedValue)}}
        SettingRow{label:root.t("Mute microphone","كتم الميكروفون");Switch{checked:!!System.state.inputMuted;enabled:!!System.state.inputVolumeAvailable&&!System.busy;onToggled:System.action("input-mute")}}
       }
       Group{SettingRow{label:root.t("Advanced audio","إعدادات الصوت المتقدمة");hint:root.t("Per-application routing and hardware profiles","توجيه صوت التطبيقات وأوضاع الأجهزة");HarborButton{text:root.t("Mixer…","مدير الصوت…");onClicked:System.openTool("audio")}}}
      }
      ColumnLayout{visible:root.section==="Accessibility";Layout.fillWidth:true;spacing:18
       Group{SettingRow{label:root.t("Reduce motion","تقليل الحركة");hint:root.t("Reduce animation in Harbor","تقليل الحركات في Harbor");Switch{checked:Prefs.reduceMotion;onToggled:Prefs.reduceMotion=checked}}}
       Group{SettingRow{label:root.t("Reduce transparency","تقليل الشفافية");Switch{checked:Prefs.opacity===1;onToggled:{if(checked){root.previousOpacity=Prefs.opacity;Prefs.opacity=1}else Prefs.opacity=root.previousOpacity}}}}
       Note{text:root.t("These controls apply to Harbor. Screen-reader, magnifier and assistive-input configuration is not included yet.","تخص هذه الخيارات Harbor. إعدادات قارئ الشاشة والمكبّر والإدخال المساعد غير مدمجة بعد.")}
      }
      ColumnLayout{visible:root.section==="Desktop & Dock";Layout.fillWidth:true;spacing:18
       Group{Image{Layout.fillWidth:true;Layout.preferredHeight:170;source:"qrc:/assets/wallpapers/"+Prefs.wallpaper+".svg";fillMode:Image.PreserveAspectCrop;clip:true}Note{text:root.t("Harbor • Original artwork","Harbor • خلفية أصلية")}}
       Group{SettingRow{label:root.t("Wallpaper","الخلفية");Select{model:["Harbor","Sunset","Forest"];property var keys:["harbor","sunset","forest"];currentIndex:keys.indexOf(Prefs.wallpaper);onActivated:Prefs.wallpaper=keys[currentIndex]}}Divider{}SettingRow{label:root.t("Dock icon size","حجم أيقونات Dock");Slider{from:32;to:56;stepSize:2;Layout.preferredWidth:190;value:Prefs.dockIconSize;onMoved:Prefs.dockIconSize=Math.round(value)}}}
       Group{SettingRow{label:root.t("Panel opacity","عتامة البانل");Label{Layout.preferredWidth:60;text:Math.round(Prefs.opacity*100)+"%"}}Slider{Layout.fillWidth:true;from:.45;to:1;value:Prefs.opacity;onMoved:Prefs.opacity=value}}
       Group{Label{text:root.t("Pinned applications","التطبيقات المثبتة في Dock");font.bold:true}Note{text:root.t("Right-click an app in the launcher to pin or unpin it. Press and hold a pinned Dock icon to remove it.","اضغط بزر الفأرة الأيمن على تطبيق في قائمة التطبيقات لتثبيته أو إلغاء تثبيته. اضغط مطوّلًا على أيقونته في Dock لإزالتها.")}}
      }
      ColumnLayout{visible:root.section==="Displays";Layout.fillWidth:true;spacing:18
       Group{SettingRow{label:root.t("Brightness","السطوع");Slider{Layout.preferredWidth:200;from:5;to:100;enabled:!!System.state.brightnessAvailable;value:System.state.brightnessPercent||0;onMoved:{brightnessTimer.requestedValue=value;brightnessTimer.restart()}Timer{id:brightnessTimer;property real requestedValue:0;interval:180;onTriggered:System.action("brightness",Math.round(requestedValue))}}}}
       Repeater{model:System.state.displayOutputs||[];delegate:Group{required property var modelData;visible:modelData.connected
        Label{text:modelData.name;font.bold:true;font.pixelSize:17}
        SettingRow{label:root.t("Scale","التحجيم");Select{id:scale;model:["100%","125%","150%","175%","200%"];currentIndex:Math.max(0,Math.round((modelData.scale-1)*4))}}
        SettingRow{label:root.t("Resolution","الدقة");Select{id:mode;Layout.preferredWidth:230;model:modelData.modes||[];textRole:"name";currentIndex:(modelData.modes||[]).findIndex(m=>String(m.id)===String(modelData.currentModeId))}}
        HarborButton{text:root.t("Apply","تطبيق");Layout.alignment:Qt.AlignRight;enabled:!System.state.displayChanging;onClicked:System.applyDisplay(modelData.id,1+scale.currentIndex*.25,mode.currentIndex>=0?String(modelData.modes[mode.currentIndex].id):"")}
       }}
       Note{visible:!(System.state.displayOutputs||[]).length;text:root.t("Display information is unavailable. Install kscreen to enable resolution controls.","معلومات الشاشات غير متاحة. ثبّت kscreen لتفعيل التحكم بالدقة.")}
       Note{text:root.t("Display changes revert after 15 seconds unless confirmed.","تُستعاد إعدادات الشاشة بعد 15 ثانية ما لم تؤكد التغيير.")}
       HarborButton{text:root.t("Keep changes","الاحتفاظ بالتغييرات");visible:!!System.state.displayPending;prominent:true;onClicked:System.confirmDisplay()}
      }
      ColumnLayout{id:keyboardPage;visible:root.section==="Keyboard";Layout.fillWidth:true;spacing:18
       property var selectedLayouts:[]
       property bool loaded:false
       property string selectedShortcut:""
       function reload(){if(Keyboard.state.layouts){selectedLayouts=Keyboard.state.layouts.slice();selectedShortcut=Keyboard.state.shortcut||"";loaded=true}}
       Component.onCompleted:reload()
       Connections{target:Keyboard;function onChanged(){if(!keyboardPage.loaded)keyboardPage.reload()}}
       Group{Label{text:root.t("Text Input","إدخال النص");font.bold:true;font.pixelSize:16}
        Note{text:root.t("Choose up to four input sources. The first is the default.","اختر حتى أربع لغات كتابة. اللغة الأولى هي الافتراضية.")}
        Repeater{model:keyboardPage.selectedLayouts;delegate:SettingRow{required property string modelData;required property int index;label:((Keyboard.state.catalog||[]).find(x=>x.id===modelData)||({name:modelData})).name
         HarborButton{text:"↑";enabled:index>0;Accessible.name:"Move input source up";onClicked:{let v=keyboardPage.selectedLayouts.slice();let a=v[index-1];v[index-1]=v[index];v[index]=a;keyboardPage.selectedLayouts=v}}
         HarborButton{text:"−";enabled:keyboardPage.selectedLayouts.length>1;Accessible.name:"Remove input source";onClicked:keyboardPage.selectedLayouts=keyboardPage.selectedLayouts.filter((x,i)=>i!==index)}
        }}
        Divider{}
        RowLayout{Select{id:inputCatalog;objectName:"input-catalog";Layout.fillWidth:true;model:Keyboard.state.catalog||[];textRole:"name"}HarborButton{objectName:"add-input";text:"+";Accessible.name:"Add input source";enabled:keyboardPage.selectedLayouts.length<4&&inputCatalog.currentIndex>=0&&!keyboardPage.selectedLayouts.includes((Keyboard.state.catalog||[])[inputCatalog.currentIndex].id);onClicked:keyboardPage.selectedLayouts=keyboardPage.selectedLayouts.concat([Keyboard.state.catalog[inputCatalog.currentIndex].id])}}
       }
       Group{SettingRow{label:root.t("Switch input source","تبديل لغة الكتابة");Select{Layout.preferredWidth:190;model:["None","Alt + Shift","Ctrl + Shift","Super + Space","Ctrl + Space"];property var values:["","grp:alt_shift_toggle","grp:ctrl_shift_toggle","grp:win_space_toggle","grp:ctrl_space_toggle"];currentIndex:Math.max(0,values.indexOf(keyboardPage.selectedShortcut));onActivated:keyboardPage.selectedShortcut=values[currentIndex]}}
        Note{text:root.t("Super is the Windows or Command key. Existing layout variants and unrelated keyboard options are preserved.","Super هو مفتاح Windows أو Command. تُحفظ تنويعات التخطيط وخيارات المفاتيح الأخرى الحالية.")}}
       RowLayout{HarborButton{objectName:"apply-keyboard";text:root.t("Apply","تطبيق");prominent:true;enabled:!Keyboard.busy&&keyboardPage.selectedLayouts.length>0;onClicked:Keyboard.apply(keyboardPage.selectedLayouts,keyboardPage.selectedShortcut)}HarborButton{text:root.t("Switch now","تبديل الآن");enabled:!Keyboard.busy;onClicked:Keyboard.switchNext()}Item{Layout.fillWidth:true}}
       HarborField{Layout.fillWidth:true;placeholderText:root.t("Type here to test your keyboard…","اكتب هنا لتجربة لوحة المفاتيح…")}
       Note{text:Keyboard.message;visible:text.length>0}
      }
      ColumnLayout{visible:root.section==="Battery";Layout.fillWidth:true;spacing:18
       Group{visible:!!System.state.batteryAvailable;SettingRow{label:root.t("Battery","البطارية");hint:System.state.batteryCharging?root.t("Charging","جارٍ الشحن"):root.t("On battery / fully charged","على البطارية / مكتملة الشحن");Label{text:Math.round(System.state.batteryPercent||0)+"%";Layout.preferredWidth:70}}}
       Group{Label{text:root.t("Energy mode","وضع الطاقة");font.bold:true}Note{text:root.t("Current mode: ","الوضع الحالي: ")+(System.state.power||root.t("Unavailable","غير متاح"))}
        Repeater{model:[{id:"power-saver",en:"Low Power",ar:"توفير الطاقة"},{id:"balanced",en:"Balanced",ar:"متوازن"},{id:"performance",en:"Performance",ar:"الأداء"}].filter(x=>(System.state.powerProfiles||[]).includes(x.id));delegate:RadioButton{required property var modelData;text:root.t(modelData.en,modelData.ar);checked:System.state.power===modelData.id;enabled:!!System.state.powerAvailable&&!System.busy;onClicked:System.action("power",modelData.id)}}}
       Note{text:root.t("Available modes depend on your hardware and power service.","تعتمد الأوضاع المتاحة على جهازك وخدمة إدارة الطاقة.")}
      }
      ColumnLayout{visible:root.section==="Users & Groups";Layout.fillWidth:true;spacing:18
       Note{text:Accounts.error;visible:text.length>0}
       Repeater{model:Accounts.users;delegate:Group{required property var modelData;Label{text:modelData.UserName;font.bold:true;font.pixelSize:16}Note{text:modelData.AccountType===1?root.t("Administrator","مسؤول"):root.t("Standard account","حساب عادي")}
        SettingRow{label:root.t("Full name","الاسم الكامل");HarborField{id:realName;text:modelData.RealName;Layout.preferredWidth:200}HarborButton{text:root.t("Save","حفظ");enabled:!Accounts.busy;onClicked:Accounts.setRealName(modelData.path,realName.text)}}}}
       Note{text:root.t("Changing a name uses system authentication. Account creation and password changes are not included yet.","تغيير الاسم يستخدم مصادقة النظام. إنشاء الحسابات وتغيير كلمات المرور غير مدمجين بعد.")}
      }
      ColumnLayout{visible:root.section==="Software Update";Layout.fillWidth:true;spacing:18
       Group{Label{text:root.t("Debian software updates","تحديثات برامج Debian");font.bold:true}Note{text:root.t("Updates are managed by the system software manager, with its own authentication.","تُدار التحديثات عبر مدير برامج النظام ومصادقته.")}HarborButton{text:root.t("Open software manager…","فتح مدير البرامج…");onClicked:System.openTool("updates")}}
      }
      ColumnLayout{visible:root.section==="About";Layout.fillWidth:true;spacing:18
       Rectangle{width:80;height:80;radius:18;Layout.alignment:Qt.AlignHCenter;color:"#4888db";Text{anchors.centerIn:parent;text:"◈";color:"white";font.pixelSize:60}}
       Label{text:"Harbor Desktop";font.pixelSize:26;font.bold:true;horizontalAlignment:Text.AlignHCenter}
       Note{text:root.t("Version ","الإصدار ")+HarborVersion;horizontalAlignment:Text.AlignHCenter}
       Group{SettingRow{label:root.t("Window system","نظام النوافذ");Label{text:"KWin · Wayland";Layout.preferredWidth:170}}Divider{}SettingRow{label:root.t("Interface","الواجهة");Label{text:"Harbor · Qt 6";Layout.preferredWidth:170}}Divider{}SettingRow{label:root.t("License","الترخيص");Label{text:"GPL-3.0-or-later";Layout.preferredWidth:170}}}
       Group{SettingRow{label:root.t("Operating system","نظام التشغيل");Label{text:System.state.osName||"Debian Linux";Layout.preferredWidth:220}}Divider{}SettingRow{label:root.t("Architecture","البنية");Label{text:System.state.architecture||"—";Layout.preferredWidth:220}}}
       Note{text:root.t("An independent open-source desktop with original artwork. Plasma Shell is not used.","سطح مكتب مستقل مفتوح المصدر بأصول أصلية. لا يستخدم Plasma Shell.")}
      }
      Note{text:System.message;visible:text.length>0;color:Prefs.dark?"#e0b471":"#986318"}
     }
    }
   }
  }
 }
 MouseArea{anchors.right:parent.right;anchors.bottom:parent.bottom;width:16;height:16;cursorShape:Qt.SizeFDiagCursor;onPressed:UI.windowAction("resize")}
}
