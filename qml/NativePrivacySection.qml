import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout{
 id:root;spacing:12
 property var catalog:UserSettings.state.sandbox||({applications:[],available:false,wayland:false})
 property string selectedId:"";property bool dirty:false;property var draft:({});property int polls:0
 property var selected:catalog.applications.find(a=>a.id===selectedId)||({})
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 function load(){draft=Object.assign({},selected.policy||{});dirty=false}
 function change(k,v){let p=Object.assign({},draft);p[k]=v;draft=p;dirty=true}
 onSelectedChanged:if(!dirty)load()
 onCatalogChanged:if(!selectedId&&catalog.applications.length)selectedId=catalog.applications[0].id
 Component.onCompleted:UserSettings.request("sandbox-list")
 Timer{interval:2000;running:root.polls>0;repeat:true;onTriggered:if(!UserSettings.busy&&!root.dirty){--root.polls;UserSettings.request("sandbox-list")}}
 HarborLabel{text:root.t("Native application isolation","عزل التطبيقات العادية");font.bold:true;font.pixelSize:17}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.t("Optional isolation for compatible Wayland applications. Each application gets a separate home folder. These controls apply to isolated launches from Harbor, not an already running application or launches from a terminal. D-Bus, X11, GPU acceleration and file portals are not passed through; some applications will not work.","عزل اختياري لتطبيقات Wayland المتوافقة، بمجلد شخصي منفصل لكل تطبيق. تنطبق الأذونات على التشغيل المعزول من Harbor، ولا تغيّر التطبيقات المفتوحة أو المشغّلة من الطرفية. لا يُمرّر D-Bus أو X11 أو تسريع الرسوميات أو بوابات الملفات؛ وقد لا تعمل بعض التطبيقات.")}
 HarborLabel{visible:!root.catalog.available;Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.t("Install bubblewrap to use isolation. If isolation fails, Harbor will not launch the application without it.","ثبّت bubblewrap لاستخدام العزل. إذا فشل العزل فلن يشغّل Harbor التطبيق دونه.")}
 ComboBox{Layout.fillWidth:true;model:root.catalog.applications;textRole:"name";valueRole:"id";currentIndex:model.findIndex(x=>x.id===root.selectedId);onActivated:{root.dirty=false;root.selectedId=currentValue;root.load()}}
 Repeater{model:[{key:"enabled",en:"Use isolation for launches from Harbor",ar:"استخدام العزل عند التشغيل من Harbor"},{key:"network",en:"Network including host-local services",ar:"الشبكة بما فيها خدمات الجهاز المحلية"},{key:"audio",en:"Audio and microphone",ar:"الصوت والميكروفون"},{key:"camera",en:"Camera devices",ar:"أجهزة الكاميرا"}];delegate:RowLayout{required property var modelData;Layout.fillWidth:true;HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.t(modelData.en,modelData.ar)}Switch{checked:!!root.draft[modelData.key];enabled:root.selectedId.length>0&&!UserSettings.busy;onClicked:{root.change(modelData.key,checked);checked=Qt.binding(()=>!!root.draft[modelData.key])}}}}
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.t("Network access also exposes local and abstract sockets and weakens isolation. Audio access includes recording; the shared Wayland connection still allows desktop interaction.","إتاحة الشبكة تتيح أيضًا المقابس المحلية وتضعف العزل. إذن الصوت يشمل التسجيل، واتصال Wayland المشترك يظل يتيح التفاعل مع سطح المكتب.")}
 Repeater{model:[{key:"documents",en:"Documents",ar:"المستندات"},{key:"downloads",en:"Downloads",ar:"التنزيلات"},{key:"pictures",en:"Pictures",ar:"الصور"}];delegate:RowLayout{required property var modelData;Layout.fillWidth:true;HarborLabel{Layout.fillWidth:true;text:root.t(modelData.en,modelData.ar)}ComboBox{model:[root.t("No access","بلا وصول"),root.t("Read only","قراءة فقط"),root.t("Read and write","قراءة وكتابة")];currentIndex:["none","read","write"].indexOf(root.draft[modelData.key]||"none");enabled:root.selectedId.length>0&&!UserSettings.busy;onActivated:root.change(modelData.key,["none","read","write"][currentIndex])}}}
 Flow{Layout.fillWidth:true;spacing:8
  HarborButton{text:root.t("Save isolation policy","حفظ إعدادات العزل");enabled:root.dirty&&!UserSettings.busy;onClicked:{UserSettings.request("sandbox-save",{id:root.selectedId,policy:root.draft});root.dirty=false}}
  HarborButton{text:root.t("Launch isolated","تشغيل معزول");enabled:root.selectedId.length>0&&!root.dirty&&!UserSettings.busy&&root.catalog.available&&root.catalog.wayland;onClicked:{UserSettings.request("sandbox-start",{id:root.selectedId});root.polls=8}}
  HarborButton{text:root.t("Refresh status","تحديث الحالة");enabled:!UserSettings.busy;onClicked:UserSettings.request("sandbox-list")}
 }
 HarborLabel{Layout.fillWidth:true;wrapMode:Text.Wrap;text:root.selected.status||"";textFormat:Text.PlainText}
}
