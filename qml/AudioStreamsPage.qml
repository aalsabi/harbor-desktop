pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id:root;spacing:14
 property color ink:Prefs.dark?"#eeeef0":"#252527"
 property color muted:Prefs.dark?"#a6a6ad":"#76767c"
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 Component.onCompleted:AudioStreams.setActive(true)
 Component.onDestruction:AudioStreams.setActive(false)
 Text{Layout.fillWidth:true;text:root.t("Applications","التطبيقات");color:root.ink;font.bold:true;font.pixelSize:16}
 Text{Layout.fillWidth:true;wrapMode:Text.WordWrap;color:root.muted;text:AudioStreams.error||(!AudioStreams.available?root.t("Application audio controls need the PulseAudio-compatible PipeWire service and pactl.","تحتاج إعدادات صوت التطبيقات إلى خدمة PipeWire المتوافقة مع PulseAudio وأداة pactl."):AudioStreams.streams.length===0?root.t("No applications are playing audio.","لا توجد تطبيقات تشغّل الصوت."):root.t("Adjust each active audio stream and choose its output device.","اضبط صوت كل تطبيق واختر جهاز الإخراج."))}
 Repeater{model:AudioStreams.streams;delegate:Rectangle{
  id:card;required property var modelData
  Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7"
  ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:10
   Text{Layout.fillWidth:true;text:card.modelData.name;color:root.ink;font.bold:true;elide:Text.ElideRight}
   Text{Layout.fillWidth:true;visible:card.modelData.description.length>0;text:card.modelData.description;color:root.muted;elide:Text.ElideRight}
   RowLayout{Layout.fillWidth:true
    Slider{id:volume;objectName:"stream-volume-"+card.modelData.id;Layout.fillWidth:true;from:0;to:100;stepSize:1;value:card.modelData.volume;enabled:!AudioStreams.busy&&card.modelData.canVolume;Accessible.name:root.t("Application volume","مستوى صوت التطبيق");property int requested:0
     onMoved:{requested=Math.round(value);if(!pressed)commit.restart()}
     onPressedChanged:{if(pressed){requested=Math.round(value);AudioStreams.setActive(false)}else{if(enabled)AudioStreams.setVolume(card.modelData.id,requested);AudioStreams.setActive(true)}}
     Timer{id:commit;interval:200;onTriggered:AudioStreams.setVolume(card.modelData.id,volume.requested)}
    }
    Text{text:Math.round(volume.value)+"%";color:root.muted;Layout.preferredWidth:40}
    Switch{text:root.t("Mute","كتم");checked:card.modelData.muted;enabled:!AudioStreams.busy;onClicked:{AudioStreams.setMuted(card.modelData.id,checked);checked=Qt.binding(()=>card.modelData.muted)}}
   }
   ComboBox{Layout.fillWidth:true;model:AudioStreams.sinks;textRole:"name";currentIndex:AudioStreams.sinks.findIndex(s=>s.id===card.modelData.sink);enabled:!AudioStreams.busy&&count>0;Accessible.name:root.t("Output device","جهاز الإخراج");onActivated:AudioStreams.move(card.modelData.id,AudioStreams.sinks[currentIndex].id)}
  }
 }}
 HarborButton{text:root.t("Refresh applications","تحديث التطبيقات");enabled:!AudioStreams.busy;onClicked:AudioStreams.refresh()}
}
