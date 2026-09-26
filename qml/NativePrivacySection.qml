import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout{
 id:root;spacing:12
 property var catalog:UserSettings.state.sandbox||({applications:[],available:false,wayland:false})
 property string selectedId:"";property bool dirty:false;property var draft:({});property int polls:0
 property var selected:catalog.applications.find(a=>a.id===selectedId)||({})
 function load(){draft=Object.assign({},selected.policy||{});dirty=false}
 function change(k,v){let p=Object.assign({},draft);p[k]=v;draft=p;dirty=true}
 onSelectedChanged:if(!dirty)load()
 onCatalogChanged:if(!selectedId&&catalog.applications.length)selectedId=catalog.applications[0].id
 Component.onCompleted:UserSettings.request("sandbox-list")
 Timer{interval:2000;running:root.polls>0;repeat:true;onTriggered:if(!UserSettings.busy&&!root.dirty){--root.polls;UserSettings.request("sandbox-list")}}
 HarborLabel{text:qsTr("Native application isolation");font.bold:true;font.pixelSize:17}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:qsTr("Optional isolation for compatible Wayland applications. Each application gets a separate home folder. These controls apply to isolated launches from Harbor, not an already running application or launches from a terminal. D-Bus, X11, GPU acceleration and file portals are not passed through; some applications will not work.")}
 HarborLabel{visible:!root.catalog.available;Layout.fillWidth:true;wrapMode:Text.Wrap;text:qsTr("Install bubblewrap to use isolation. If isolation fails, Harbor will not launch the application without it.")}
 ComboBox{Layout.fillWidth:true;model:root.catalog.applications;textRole:"name";valueRole:"id";currentIndex:model.findIndex(x=>x.id===root.selectedId);onActivated:{root.dirty=false;root.selectedId=currentValue;root.load()}}
 Repeater{model:[{key:"enabled",en:QT_TR_NOOP("Use isolation for launches from Harbor")},{key:"network",en:QT_TR_NOOP("Network including host-local services")},{key:"audio",en:QT_TR_NOOP("Audio and microphone")},{key:"camera",en:QT_TR_NOOP("Camera devices")}];delegate:RowLayout{required property var modelData;Layout.fillWidth:true;HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:qsTr(modelData.en)}Switch{checked:!!root.draft[modelData.key];enabled:root.selectedId.length>0&&!UserSettings.busy;onClicked:{root.change(modelData.key,checked);checked=Qt.binding(()=>!!root.draft[modelData.key])}}}}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:qsTr("Network access also exposes local and abstract sockets and weakens isolation. Audio access includes recording; the shared Wayland connection still allows desktop interaction.")}
 Repeater{model:[{key:"documents",en:QT_TR_NOOP("Documents")},{key:"downloads",en:QT_TR_NOOP("Downloads")},{key:"pictures",en:QT_TR_NOOP("Pictures")}];delegate:RowLayout{required property var modelData;Layout.fillWidth:true;HarborLabel{Layout.fillWidth:true;text:qsTr(modelData.en)}ComboBox{model:[qsTr("No access"),qsTr("Read only"),qsTr("Read and write")];currentIndex:["none","read","write"].indexOf(root.draft[modelData.key]||"none");enabled:root.selectedId.length>0&&!UserSettings.busy;onActivated:root.change(modelData.key,["none","read","write"][currentIndex])}}}
 Flow{Layout.fillWidth:true;spacing:8
  HarborButton{text:qsTr("Save isolation policy");enabled:root.dirty&&!UserSettings.busy;onClicked:{UserSettings.request("sandbox-save",{id:root.selectedId,policy:root.draft});root.dirty=false}}
  HarborButton{text:qsTr("Launch isolated");enabled:root.selectedId.length>0&&!root.dirty&&!UserSettings.busy&&root.catalog.available&&root.catalog.wayland;onClicked:{UserSettings.request("sandbox-start",{id:root.selectedId});root.polls=8}}
  HarborButton{text:qsTr("Refresh status");enabled:!UserSettings.busy;onClicked:UserSettings.request("sandbox-list")}
 }
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.selected.status||"";textFormat:Text.PlainText}
}
