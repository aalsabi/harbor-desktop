pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
 id: root
 objectName: "languageRegionPage"
 spacing: 18
 signal keyboardRequested()
 property var draft: ({})
 property var advancedDraft: ({})
 property int selectedLanguage: 0
 property string status: ""
 property bool ready: false
 readonly property bool arabic: Prefs.language === "ar"
 readonly property color ink: Prefs.dark ? "#eeeeef" : "#26262a"
 readonly property color muted: Prefs.dark ? "#aaaab2" : "#696971"
 readonly property color line: Prefs.dark ? "#505057" : "#d2d2d8"
 readonly property var sample: ready ? Region.preview(draft) : ({})
 readonly property var advancedSample: advanced.visible ? Region.preview(advancedDraft) : ({})
 property var activePatternField: null
 LayoutMirroring.enabled: arabic
 LayoutMirroring.childrenInherit: true
 function t(en, ar) { return arabic ? ar : en }
 function copy(value) { return JSON.parse(JSON.stringify(value)) }
 function stage(key, value) { var next=copy(draft); next[key]=value; draft=next; status="" }
 function stageAdvanced(key, value) { var next=copy(advancedDraft); next[key]=value; advancedDraft=next }
 function language(code) { return Region.languages.find(function(l){return l.code===code}) || ({code:code,name:code,nativeName:code}) }
 function languageTitle(code) { var l=language(code); return l.nativeName === l.name ? l.name : l.nativeName + " — " + l.name }
 function setHour24(enabled) {
  var next=copy(draft);next.hour24=enabled
  next.timeFormats=next.timeFormats.map(function(pattern){
   var parts=pattern.split(/('(?:[^']|'')*')/g);var hasPeriod=false
   for(var i=0;i<parts.length;i+=2){
    parts[i]=parts[i].replace(/AP|ap|H{1,2}|h{1,2}/g,function(token){
     if(token.toLowerCase()==="ap"){hasPeriod=true;return enabled?"":token}
     return enabled?"HH":(token.length===2?"hh":"h")
    })
   }
   var result=parts.join("").trim();return !enabled&&!hasPeriod?result+" AP":result
  });draft=next;status=""
 }
 function changeRegion(code) {
  var next=copy(Region.defaults(code));next.languages=draft.languages.slice();draft=next;status=""
 }
 function moveLanguage(offset) {
  var values=draft.languages.slice(); var destination=selectedLanguage+offset
  if(destination<0 || destination>=values.length)return
  var value=values.splice(selectedLanguage,1)[0]; values.splice(destination,0,value)
  stage("languages",values);selectedLanguage=destination
 }
 function restoreMain() {
  var next=copy(Region.defaults(draft.region));next.languages=draft.languages.slice();draft=next
  status=t("Defaults restored in this draft. Choose Apply to save.","تمت استعادة الإعدادات الافتراضية في المسودة. اختر تطبيق لحفظها.")
 }
 function restoreAdvanced() {
  var defaults=Region.defaults(advancedDraft.region);var next=copy(advancedDraft)
  var keys=tabs.currentIndex===1?["dateFormats"]:tabs.currentIndex===2?["timeFormats","am","pm","hour24"]:["formatLanguage","numberGroup","numberDecimal","currency","currencyGroup","currencyDecimal","measurement"]
  keys.forEach(function(key){next[key]=defaults[key]});advancedDraft=next
 }
 function openAdvanced() { advancedDraft=copy(draft); tabs.currentIndex=0; activePatternField=null; advanced.open() }
 function setPattern(index, value) {
  var key=tabs.currentIndex===1?"dateFormats":"timeFormats"
  var values=(advancedDraft[key] || []).slice(); values[index]=value; stageAdvanced(key,values)
  if(key==="timeFormats" && index===0)stageAdvanced("hour24", !/AP|ap/.test(value.replace(/'(?:[^']|'')*'/g,"")))
 }
 function insertToken(token) {
  if(!activePatternField)return
  var field=activePatternField;field.insert(field.cursorPosition,token);setPattern(field.patternIndex,field.text);field.forceActiveFocus()
 }
 property var generationDraft: ({})
 property var requiredLocales: []
 function saved() {
  draft=copy(Region.state)
  var supported=draft.languages.find(function(code){return code.split(/[-_]/)[0]==="ar" || code.split(/[-_]/)[0]==="en"})
  if(supported)Prefs.language=supported.split(/[-_]/)[0]
  status=t("Preferences saved. Sign out and back in to apply them to other applications.","تم حفظ التفضيلات. سجّل الخروج والدخول لتطبيقها على البرامج الأخرى.")
 }
 function applyChanges() {
  if(Region.busy)return
  var plan=Region.generationPlan(draft)
  if(plan.error){status=plan.error;return}
  if(plan.locales.length){generationDraft=copy(draft);requiredLocales=plan.locales;status="";generateDialog.open();return}
  if(Region.apply(draft))saved()
  else status=Region.message || t("The settings could not be applied.","تعذر تطبيق الإعدادات.")
 }
 Connections {
  target:Region
  function onGenerationFinished(success){
   if(success){generateDialog.close();root.saved()}
   else root.status=Region.message || root.t("Generation was cancelled or failed. Your settings were not changed.","أُلغي التوليد أو تعذّر. لم تتغير إعداداتك.")
  }
 }
 Component.onCompleted: { draft=copy(Region.state); ready=true }

 component Note: HarborLabel { Layout.fillWidth:true; wrapMode:Text.WordWrap; color:root.muted; font.pixelSize:12 }
 component Heading: HarborLabel { font.bold:true; font.pixelSize:13; Layout.fillWidth:true; wrapMode:Text.WordWrap }
 component Choice: ComboBox {
  id:choice
  property var choices: []
  property string selected: ""
  signal chosen(string value)
  model: choices; textRole:"name"; valueRole:"code"
  Layout.fillWidth:true; implicitHeight:30
  currentIndex: { for(var i=0;i<choices.length;i++)if(String(choices[i].code)===selected)return i;return -1 }
  onActivated: chosen(String(currentValue))
  Accessible.name: displayText
  leftPadding:root.arabic?32:10;rightPadding:root.arabic?10:32
  contentItem:Text {text:choice.displayText;color:root.ink;font.pixelSize:13;verticalAlignment:Text.AlignVCenter;elide:Text.ElideRight;horizontalAlignment:root.arabic?Text.AlignRight:Text.AlignLeft}
  indicator:Text {x:root.arabic?10:choice.width-width-10;y:(choice.height-height)/2;text:"⌄";color:root.muted;font.pixelSize:17}
  background:Rectangle {radius:6;color:Prefs.dark?"#3c3c42":"#ffffff";border.color:choice.activeFocus?Prefs.accent:root.line}
  delegate:ItemDelegate {
   id:option
   required property int index;required property var modelData
   width:choice.width;height:34;highlighted:choice.highlightedIndex===index
   contentItem:Text {text:option.modelData.name;color:option.highlighted?"white":root.ink;font.pixelSize:13;verticalAlignment:Text.AlignVCenter;elide:Text.ElideRight}
   background:Rectangle {radius:4;color:option.highlighted?Prefs.accent:"transparent"}
  }
  popup:Popup {
   y:choice.height+4;width:choice.width;padding:5
   implicitHeight:Math.min(contentItem.implicitHeight+10,280)
   background:Rectangle {radius:7;color:Prefs.dark?"#323238":"#ffffff";border.color:root.line}
   contentItem:ListView {clip:true;implicitHeight:contentHeight;model:choice.popup.visible?choice.delegateModel:null;currentIndex:choice.highlightedIndex;ScrollBar.vertical:ScrollBar {}}
  }
 }
 component ThemedDialog: Dialog {
  id:themedDialog
  padding:18;spacing:12
  background:Rectangle {radius:12;color:Prefs.dark?"#29292e":"#f7f7f9";border.color:root.line}
  header:HarborLabel {text:themedDialog.title;font.bold:true;font.pixelSize:14;padding:18;bottomPadding:6;wrapMode:Text.WordWrap;horizontalAlignment:root.arabic?Text.AlignRight:Text.AlignLeft}
 }
 component SegmentTab: TabButton {
  id:segment
  implicitHeight:30
  contentItem:Text {text:segment.text;font.pixelSize:13;color:segment.checked?"white":root.ink;horizontalAlignment:Text.AlignHCenter;verticalAlignment:Text.AlignVCenter}
  background:Rectangle {radius:6;color:segment.checked?Prefs.accent:"transparent"}
 }
 component Pane: Frame {
  padding:14
  background:Rectangle {color:Prefs.dark?"#2e2e33":"#f7f7f9";radius:8;border.color:root.line}
 }

 GridLayout {
  Layout.fillWidth:true
  columns:root.width>=600?2:1;columnSpacing:18;rowSpacing:18
  Pane {
   Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
   Layout.preferredWidth:root.width>=600?root.width*0.47:root.width
   contentItem:ColumnLayout {
    spacing:9
    Heading {text:root.t("Preferred languages","اللغات المفضلة")}
    Note {text:root.t("Menu languages, not keyboard layouts. Keep only English for English-only menus.","هذه لغات القوائم وليست لغات الكتابة. أبقِ الإنجليزية وحدها لقوائم إنجليزية فقط.")}
    Rectangle {
     Layout.fillWidth:true;Layout.preferredHeight:184;color:Prefs.dark?"#222226":"white";border.color:root.line;radius:4
     ListView {
      id:preferred;objectName:"preferredLanguages";anchors.fill:parent;anchors.margins:1;clip:true
      model:root.draft.languages || [];currentIndex:root.selectedLanguage
      ScrollBar.vertical:ScrollBar {}
      delegate:ItemDelegate {
       id:languageRow
       required property string modelData;required property int index
       width:ListView.view.width;height:48;highlighted:index===root.selectedLanguage
       onClicked:root.selectedLanguage=index
       Accessible.name:root.languageTitle(modelData)+(index===0?root.t(", Primary","، الأساسية"):"")
       background:Rectangle {color:languageRow.highlighted?Prefs.accent:"transparent"}
       contentItem:Column {
        spacing:2
        Text {width:parent.width;text:root.languageTitle(modelData);color:languageRow.highlighted?"white":root.ink;elide:Text.ElideRight;font.pixelSize:13}
        Text {text:index===0?root.t("Primary","الأساسية"):root.t("Alternative","بديلة");color:languageRow.highlighted?"#eeffffff":root.muted;font.pixelSize:11}
       }
      }
     }
    }
    RowLayout {
     spacing:5
     HarborButton {objectName:"addLanguage";text:"+";Layout.preferredWidth:34;Accessible.name:root.t("Add language","إضافة لغة");onClicked:{languageSearch.text="";addDialog.selectedCode="";addDialog.open()}}
     HarborButton {objectName:"removeLanguage";text:"−";Layout.preferredWidth:34;enabled:(root.draft.languages || []).length>1;Accessible.name:root.t("Remove selected language","إزالة اللغة المحددة");onClicked:{var a=root.draft.languages.slice();a.splice(root.selectedLanguage,1);root.stage("languages",a);root.selectedLanguage=Math.min(root.selectedLanguage,a.length-1)}}
     Item {Layout.fillWidth:true}
     HarborButton {text:"↑";Layout.preferredWidth:34;enabled:root.selectedLanguage>0;Accessible.name:root.t("Move selected language up","نقل اللغة إلى أعلى");onClicked:root.moveLanguage(-1)}
     HarborButton {text:"↓";Layout.preferredWidth:34;enabled:root.selectedLanguage<(root.draft.languages || []).length-1;Accessible.name:root.t("Move selected language down","نقل اللغة إلى أسفل");onClicked:root.moveLanguage(1)}
    }
   }
  }
  ColumnLayout {
   Layout.fillWidth:true;Layout.alignment:Qt.AlignTop;spacing:10
   Heading {text:root.t("Region","المنطقة")}
   Choice {objectName:"regionChoice";choices:Region.regions;selected:root.draft.region || "";Accessible.name:root.t("Region","المنطقة");onChosen:root.changeRegion(value)}
   Heading {text:root.t("First day of the week","أول أيام الأسبوع")}
   Choice {choices:[{code:"1",name:root.t("Monday","الاثنين")},{code:"2",name:root.t("Tuesday","الثلاثاء")},{code:"3",name:root.t("Wednesday","الأربعاء")},{code:"4",name:root.t("Thursday","الخميس")},{code:"5",name:root.t("Friday","الجمعة")},{code:"6",name:root.t("Saturday","السبت")},{code:"7",name:root.t("Sunday","الأحد")}];selected:String(root.draft.firstDay || 1);Accessible.name:root.t("First day of the week","أول أيام الأسبوع");onChosen:root.stage("firstDay",Number(value))}
   Heading {text:root.t("Calendar","التقويم")}
   Choice {choices:Region.calendars;selected:root.draft.calendar || "Gregorian";Accessible.name:root.t("Calendar","التقويم");onChosen:root.stage("calendar",value)}
   CheckBox {objectName:"hour24";palette.windowText:root.ink;palette.text:root.ink;palette.buttonText:root.ink;text:root.t("Use 24-hour time","استخدام الوقت بنظام 24 ساعة");checked:root.draft.hour24===true;onClicked:root.setHour24(checked)}
  }
 }
 Pane {
  Layout.fillWidth:true
  contentItem:ColumnLayout {
   spacing:7
   Heading {text:root.t("Region preview","معاينة تنسيق المنطقة")}
   GridLayout {
    Layout.fillWidth:true;columns:2;columnSpacing:16;rowSpacing:6
    HarborLabel {text:root.t("Date","التاريخ");color:root.muted}
    HarborLabel {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;text:root.sample.date || ""}
    HarborLabel {text:root.t("Time","الوقت");color:root.muted}
    HarborLabel {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;text:root.sample.time || ""}
    HarborLabel {text:root.t("Number","العدد");color:root.muted}
    HarborLabel {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;text:root.sample.number || ""}
    HarborLabel {text:root.t("Week","الأسبوع");color:root.muted}
    HarborLabel {Layout.fillWidth:true;wrapMode:Text.WordWrap;text:(root.sample.weekdays || []).join(" · ")}
    HarborLabel {text:root.t("Units","الوحدات");color:root.muted}
    HarborLabel {Layout.fillWidth:true;wrapMode:Text.WordWrap;text:root.sample.measurementExample || ""}
    HarborLabel {text:root.t("Currency","العملة");color:root.muted}
    HarborLabel {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;text:root.sample.currency || ""}
   }
  }
 }
 Note {visible:!!root.sample.error;text:root.sample.error || ""}
 Note {visible:Region.localeNotice.length>0;text:Region.localeNotice}
 Note {text:root.t("Harbor is translated into English and Arabic. Preferred languages and region apply to other applications after your next login. Custom date, time and number formats apply within Harbor.","تتوفر واجهة Harbor بالعربية والإنجليزية. تسري اللغات المفضلة والمنطقة على التطبيقات الأخرى بعد تسجيل الدخول التالي. تنسيقات التاريخ والوقت والأرقام المخصصة خاصة بـ Harbor.")}
 RowLayout {
  Layout.fillWidth:true;spacing:8
  HarborButton {text:root.t("Keyboard Preferences…","تفضيلات لوحة المفاتيح…");onClicked:root.keyboardRequested()}
  Item {Layout.fillWidth:true}
  HarborButton {objectName:"advancedRegion";text:root.t("Advanced…","متقدم…");onClicked:root.openAdvanced()}
 }
 Rectangle {Layout.fillWidth:true;implicitHeight:1;color:root.line}
 RowLayout {
  Layout.fillWidth:true
  HarborButton {objectName:"restoreRegion";text:root.t("Restore Defaults","استعادة الافتراضي");onClicked:root.restoreMain()}
  Item {Layout.fillWidth:true}
  HarborButton {objectName:"revertRegion";text:root.t("Revert","تراجع");onClicked:{root.draft=root.copy(Region.state);root.selectedLanguage=0;root.status=""}}
  HarborButton {objectName:"applyRegion";text:root.t("Apply","تطبيق");prominent:true;enabled:!root.sample.error&&!Region.busy;onClicked:root.applyChanges()}
 }
 Note {objectName:"regionStatus";visible:text.length>0;text:root.status;Accessible.role:Accessible.StaticText}

 ThemedDialog {
  id:generateDialog;objectName:"generateLocalesDialog"
  parent:Overlay.overlay;anchors.centerIn:parent
  width:Math.min(500,parent?parent.width-32:500)
  modal:true;closePolicy:Region.busy?Popup.NoAutoClose:Popup.CloseOnEscape
  title:root.t("Generate regional settings?","توليد الإعدادات الإقليمية؟")
  contentItem:ColumnLayout {
   spacing:12;LayoutMirroring.enabled:root.arabic;LayoutMirroring.childrenInherit:true
   Note {text:root.t("The required locales are not generated on this computer. Choose OK to generate them and apply your preferences. Administrator authentication is required.","الإعدادات الإقليمية المطلوبة غير مولّدة على هذا الجهاز. اضغط موافق لتوليدها وتطبيق تفضيلاتك. سيطلب النظام مصادقة المسؤول.")}
   HarborLabel {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;text:root.requiredLocales.join(" · ");LayoutMirroring.enabled:false}
   RowLayout {visible:Region.busy;BusyIndicator{running:Region.busy;implicitWidth:28;implicitHeight:28}Note{text:root.t("Waiting for authentication or generating locales…","بانتظار المصادقة أو توليد الإعدادات…")}}
   Note {objectName:"generationError";visible:root.status.length>0;text:root.status}
  }
  footer:Item {
   implicitHeight:60
   RowLayout {anchors.fill:parent;anchors.margins:16;spacing:8
    Item {Layout.fillWidth:true}
    HarborButton {objectName:"cancelGeneration";text:root.t("Cancel","إلغاء");enabled:!Region.busy;onClicked:generateDialog.close()}
    HarborButton {objectName:"confirmGeneration";text:root.t("OK","موافق");prominent:true;enabled:!Region.busy;onClicked:{root.status="";Region.generateAndApply(root.generationDraft)}}
   }
  }
 }

 ThemedDialog {
  id:addDialog;objectName:"addLanguageDialog"
  parent:Overlay.overlay;anchors.centerIn:parent
  width:Math.min(480,parent?parent.width-32:480);height:Math.min(540,parent?parent.height-32:540)
  modal:true;closePolicy:Popup.CloseOnEscape
  title:root.t("Add a preferred language","إضافة لغة مفضلة")
  property string selectedCode:""
  contentItem:ColumnLayout {
   LayoutMirroring.enabled:root.arabic;LayoutMirroring.childrenInherit:true
   spacing:10
   HarborField {id:languageSearch;objectName:"languageSearch";Layout.fillWidth:true;placeholderText:root.t("Search languages","البحث عن لغة");Accessible.name:placeholderText}
   ListView {
    id:availableLanguages;Layout.fillWidth:true;Layout.fillHeight:true;clip:true
    model:Region.languages.filter(function(l){var q=languageSearch.text.toLowerCase();return (root.draft.languages || []).indexOf(l.code)<0 && (l.name+" "+l.nativeName+" "+l.code).toLowerCase().indexOf(q)>=0})
    ScrollBar.vertical:ScrollBar{}
    delegate:ItemDelegate {
     id:availableLanguage
     required property var modelData;width:ListView.view.width;height:44
     text:root.languageTitle(modelData.code);highlighted:addDialog.selectedCode===modelData.code
     contentItem:Text {text:availableLanguage.text;color:availableLanguage.highlighted?"white":root.ink;verticalAlignment:Text.AlignVCenter;elide:Text.ElideRight;font.pixelSize:13}
     background:Rectangle {radius:5;color:availableLanguage.highlighted?Prefs.accent:availableLanguage.hovered?(Prefs.dark?"#414149":"#e7e7ed"):"transparent"}
     onClicked:addDialog.selectedCode=modelData.code
    }
    HarborLabel {anchors.centerIn:parent;visible:availableLanguages.count===0;text:root.t("No matching languages","لا توجد لغات مطابقة")}
   }
  }
  footer:Frame {
   padding:16;background:Item {}
   contentItem:RowLayout {
   spacing:8
   Item {Layout.fillWidth:true}
   HarborButton {text:root.t("Cancel","إلغاء");onClicked:addDialog.close()}
   HarborButton {objectName:"confirmAddLanguage";text:root.t("Add","إضافة");prominent:true;enabled:addDialog.selectedCode.length>0;onClicked:{var a=root.draft.languages.slice();a.push(addDialog.selectedCode);root.stage("languages",a);root.selectedLanguage=a.length-1;addDialog.close()}}
     }
  }
 }
 ThemedDialog {
  id:advanced;objectName:"advancedRegionDialog"
  parent:Overlay.overlay;anchors.centerIn:parent
  width:Math.min(680,parent?parent.width-32:680);height:Math.min(690,parent?parent.height-32:690)
  modal:true;closePolicy:Popup.CloseOnEscape
  title:root.t("Advanced Language & Region","إعدادات اللغة والمنطقة المتقدمة")
  contentItem:ColumnLayout {
   LayoutMirroring.enabled:root.arabic;LayoutMirroring.childrenInherit:true
   spacing:14
   Note {visible:!!root.advancedSample.error;text:root.advancedSample.error || ""}
   TabBar {
    id:tabs;objectName:"advancedRegionTabs";Layout.fillWidth:true;implicitHeight:36;padding:3;spacing:2
    background:Rectangle {radius:9;color:Prefs.dark?"#3a3a40":"#e7e7ec";border.color:root.line}
    SegmentTab {text:root.t("General","عام")}
    SegmentTab {text:root.t("Dates","التواريخ")}
    SegmentTab {text:root.t("Times","الأوقات")}
    onCurrentIndexChanged:root.activePatternField=null
   }
   ScrollView {
    id:advancedScroll;Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
    ColumnLayout {
     width:advancedScroll.availableWidth;spacing:14
     ColumnLayout {
      visible:tabs.currentIndex===0;Layout.fillWidth:true;spacing:10
      Heading {text:root.t("Format language","لغة التنسيق")}
      Choice {choices:Region.languages;selected:root.advancedDraft.formatLanguage || "en";Accessible.name:root.t("Format language","لغة التنسيق");onChosen:root.stageAdvanced("formatLanguage",value)}
      Heading {text:root.t("Number separators","فواصل الأعداد")}
      GridLayout {
       columns:2;Layout.fillWidth:true;columnSpacing:12
       HarborLabel {text:root.t("Grouping","المجموعات")}
       HarborField {objectName:"numberGroup";Layout.fillWidth:true;text:root.advancedDraft.numberGroup || "";maximumLength:4;Accessible.name:root.t("Number grouping separator","فاصل مجموعات الأعداد");onTextEdited:root.stageAdvanced("numberGroup",text)}
       HarborLabel {text:root.t("Decimal","العشري")}
       HarborField {objectName:"numberDecimal";Layout.fillWidth:true;text:root.advancedDraft.numberDecimal || "";maximumLength:4;Accessible.name:root.t("Number decimal separator","الفاصل العشري للأعداد");onTextEdited:root.stageAdvanced("numberDecimal",text)}
      }
      Note {text:root.advancedSample.number || ""}
      Heading {text:root.t("Currency","العملة")}
      GridLayout {
       columns:2;Layout.fillWidth:true;columnSpacing:12
       HarborLabel {text:root.t("Currency","العملة")}
       Choice {objectName:"currencyCode";choices:Region.currencies;selected:root.advancedDraft.currency || "";Accessible.name:root.t("Currency","العملة");onChosen:root.stageAdvanced("currency",value)}
       HarborLabel {text:root.t("Grouping","المجموعات")}
       HarborField {Layout.fillWidth:true;text:root.advancedDraft.currencyGroup || "";maximumLength:4;Accessible.name:root.t("Currency grouping separator","فاصل مجموعات العملة");onTextEdited:root.stageAdvanced("currencyGroup",text)}
       HarborLabel {text:root.t("Decimal","العشري")}
       HarborField {Layout.fillWidth:true;text:root.advancedDraft.currencyDecimal || "";maximumLength:4;Accessible.name:root.t("Currency decimal separator","الفاصل العشري للعملة");onTextEdited:root.stageAdvanced("currencyDecimal",text)}
      }
      Note {text:root.advancedSample.currency || ""}
      Heading {text:root.t("Measurement units","وحدات القياس")}
      Choice {choices:[{code:"metric",name:root.t("Metric","متري")},{code:"us",name:root.t("US","أمريكي")},{code:"uk",name:root.t("UK","بريطاني")}];selected:root.advancedDraft.measurement || "metric";Accessible.name:root.t("Measurement units","وحدات القياس");onChosen:root.stageAdvanced("measurement",value)}
      Note {text:root.advancedSample.measurementExample || ""}
     }
     ColumnLayout {
      visible:tabs.currentIndex>0;Layout.fillWidth:true;spacing:10
      Note {text:root.t("Select a format field, then insert a component below or type a custom pattern. Previews update as you edit.","حدد حقل تنسيق ثم أدرج مكونًا من الأسفل أو اكتب نمطًا مخصصًا. تتحدث المعاينة أثناء التحرير.")}
      Repeater {
       model:4
       delegate:ColumnLayout {
        id:formatRow
        required property int index;Layout.fillWidth:true;spacing:4
        Heading {text:[root.t("Short","قصير"),root.t("Medium","متوسط"),root.t("Long","طويل"),root.t("Full","كامل")][index]}
        HarborField {
         property int patternIndex:formatRow.index
         objectName:"formatPattern"+patternIndex
         Layout.fillWidth:true;LayoutMirroring.enabled:false
         text:((tabs.currentIndex===1?root.advancedDraft.dateFormats:root.advancedDraft.timeFormats)||[])[patternIndex] || ""
         Accessible.name:(tabs.currentIndex===1?root.t("Date format ","تنسيق التاريخ "):root.t("Time format ","تنسيق الوقت "))+(patternIndex+1)
         onActiveFocusChanged:if(activeFocus)root.activePatternField=this
         onTextEdited:root.setPattern(patternIndex,text)
        }
        Note {text:((tabs.currentIndex===1?root.advancedSample.dates:root.advancedSample.times)||[])[index] || ""}
       }
      }
      Heading {text:root.t("Insert a component","إدراج مكون")}
      Flow {
       Layout.fillWidth:true;spacing:6
       Repeater {
        model:tabs.currentIndex===1?[{token:"d",name:root.t("Day","اليوم")},{token:"dddd",name:root.t("Weekday","يوم الأسبوع")},{token:"M",name:root.t("Month number","رقم الشهر")},{token:"MMMM",name:root.t("Month name","اسم الشهر")},{token:"yyyy",name:root.t("Year","السنة")}]:[{token:"HH",name:root.t("Hour 24","ساعة 24")},{token:"hh",name:root.t("Hour 12","ساعة 12")},{token:"mm",name:root.t("Minute","الدقيقة")},{token:"ss",name:root.t("Second","الثانية")},{token:"AP",name:root.t("AM/PM","ص/م")},{token:"zzz",name:root.t("Millisecond","جزء الألف")},{token:"t",name:root.t("Time zone","المنطقة الزمنية")},{token:"tttt",name:root.t("Zone name","اسم المنطقة الزمنية")}]
        delegate:HarborButton {required property var modelData;text:modelData.name+" · "+modelData.token;enabled:root.activePatternField!==null;Accessible.name:root.t("Insert ","إدراج ")+modelData.name;onClicked:root.insertToken(modelData.token)}
       }
      }
      Note {text:root.t("Use single quotes around literal words. For example: d MMMM yyyy or hh:mm AP. A 12-hour time format needs AP.","ضع الكلمات الحرفية بين علامتي اقتباس مفردتين. مثال: d MMMM yyyy أو hh:mm AP. يحتاج تنسيق 12 ساعة إلى AP.")}
      GridLayout {
       visible:tabs.currentIndex===2;Layout.fillWidth:true;columns:2;columnSpacing:12
       HarborLabel {text:root.t("Before noon","قبل الظهر")}
       HarborField {Layout.fillWidth:true;text:root.advancedDraft.am || "";Accessible.name:root.t("AM label","رمز ما قبل الظهر");onTextEdited:root.stageAdvanced("am",text)}
       HarborLabel {text:root.t("After noon","بعد الظهر")}
       HarborField {Layout.fillWidth:true;text:root.advancedDraft.pm || "";Accessible.name:root.t("PM label","رمز ما بعد الظهر");onTextEdited:root.stageAdvanced("pm",text)}
      }
     }
    }
   }
  }
  footer:Frame {
   padding:16;background:Item {}
   contentItem:RowLayout {
   spacing:8
   Item {Layout.fillWidth:true}
   HarborButton {objectName:"restoreAdvanced";text:root.t("Restore Defaults","استعادة الافتراضي");onClicked:root.restoreAdvanced()}
   HarborButton {objectName:"cancelAdvanced";text:root.t("Cancel","إلغاء");onClicked:advanced.close()}
   HarborButton {objectName:"acceptAdvanced";text:root.t("OK","موافق");prominent:true;enabled:!root.advancedSample.error;onClicked:{root.draft=root.copy(root.advancedDraft);root.status="";advanced.close()}}
     }
  }
 }
}
