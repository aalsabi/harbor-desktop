pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id:root;spacing:14
 property color ink:Prefs.dark?"#eeeef0":"#252527"
 property color muted:Prefs.dark?"#a6a6ad":"#76767c"
 Component.onCompleted:AudioStreams.setActive(true)
 Component.onDestruction:AudioStreams.setActive(false)
 Text{Layout.fillWidth:true;text:qsTr("Applications");color:root.ink;font.bold:true;font.pixelSize:16}
 Text{Layout.fillWidth:true;wrapMode:Text.WordWrap;color:root.muted;text:AudioStreams.error||(!AudioStreams.available?qsTr("Application audio controls need the PulseAudio-compatible PipeWire service and pactl."):AudioStreams.streams.length===0?qsTr("No applications are playing audio."):qsTr("Adjust each active audio stream and choose its output device."))}
 Repeater{model:AudioStreams.streams;delegate:Rectangle{
  id:card;required property var modelData
  Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7"
  ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:10
   Text{Layout.fillWidth:true;text:card.modelData.name;color:root.ink;font.bold:true;elide:Text.ElideRight}
   Text{Layout.fillWidth:true;visible:card.modelData.description.length>0;text:card.modelData.description;color:root.muted;elide:Text.ElideRight}
   RowLayout{Layout.fillWidth:true
    Slider{id:volume;objectName:"stream-volume-"+card.modelData.id;Layout.fillWidth:true;from:0;to:100;stepSize:1;value:card.modelData.volume;enabled:!AudioStreams.busy&&card.modelData.canVolume;Accessible.name:qsTr("Application volume");property int requested:0
     onMoved:{requested=Math.round(value);if(!pressed)commit.restart()}
     onPressedChanged:{if(pressed){requested=Math.round(value);AudioStreams.setActive(false)}else{if(enabled)AudioStreams.setVolume(card.modelData.id,requested);AudioStreams.setActive(true)}}
     Timer{id:commit;interval:200;onTriggered:AudioStreams.setVolume(card.modelData.id,volume.requested)}
    }
    Text{text:Math.round(volume.value)+"%";color:root.muted;Layout.preferredWidth:40}
    Switch{text:qsTr("Mute");checked:card.modelData.muted;enabled:!AudioStreams.busy;onClicked:{AudioStreams.setMuted(card.modelData.id,checked);checked=Qt.binding(()=>card.modelData.muted)}}
   }
   ComboBox{Layout.fillWidth:true;model:AudioStreams.sinks;textRole:"name";currentIndex:AudioStreams.sinks.findIndex(s=>s.id===card.modelData.sink);enabled:!AudioStreams.busy&&count>0;Accessible.name:qsTr("Output device");onActivated:AudioStreams.move(card.modelData.id,AudioStreams.sinks[currentIndex].id)}
  }
 }}
 HarborButton{text:qsTr("Refresh applications");enabled:!AudioStreams.busy;onClicked:AudioStreams.refresh()}
}
