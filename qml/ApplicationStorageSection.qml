pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id: root
 objectName: "applicationStorageSection"
 spacing: 12
 readonly property bool arabic: Prefs.language === "ar"
 readonly property color muted: Prefs.dark ? "#aaaab2" : "#696971"
 property int shown: 20
 property var pendingAssociation: ({})
 readonly property var matching: ApplicationStorage.applications.filter(function(app){return app.name.toLowerCase().indexOf(filter.text.toLowerCase())>=0})
 function t(en, ar) { return arabic ? ar : en }
 function size(value) { var amount=Number(value || 0),units=["B","KiB","MiB","GiB","TiB"],i=0;while(amount>=1024&&i<units.length-1){amount/=1024;++i}return amount.toFixed(i?1:0)+" "+units[i] }
 LayoutMirroring.enabled: arabic
 LayoutMirroring.childrenInherit: true
 component Note: HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: root.muted; textFormat: Text.PlainText }
 component Card: Rectangle {
  default property alias contents: layout.data
  Layout.fillWidth: true; implicitHeight: layout.implicitHeight+28
  radius: 12; color: Prefs.dark ? "#29292f" : "#ffffff"; border.color: Prefs.dark ? "#45454d" : "#dedee4"
  ColumnLayout { id: layout; anchors { left: parent.left; right: parent.right; top: parent.top; margins: 14 } spacing: 8 }
 }
 HarborLabel { text: root.t("Measured application storage", "مساحة التطبيقات المقاسة"); font.pixelSize: 17; font.bold: true; Layout.fillWidth: true; wrapMode: Text.WordWrap }
 Note { text: root.t("Measure installed application files, data, and cache using allocated disk blocks. Nothing is deleted. The scan starts only when you choose Measure.", "قياس ملفات التطبيقات المثبتة وبياناتها وذاكرتها المؤقتة باستخدام مساحة القرص المشغولة فعليًا. لن يُحذف شيء. يبدأ الفحص فقط عند اختيار قياس.") }
 Flow {
  Layout.fillWidth: true; spacing: 8
  HarborButton { text: root.t("Measure application storage", "قياس مساحة التطبيقات"); enabled: !ApplicationStorage.busy; onClicked: {root.shown=20;ApplicationStorage.measure()} }
  HarborButton { visible: ApplicationStorage.busy; text: root.t("Cancel scan", "إلغاء الفحص"); onClicked: ApplicationStorage.cancel() }
 }
 Note { visible: ApplicationStorage.message.length>0; text: ApplicationStorage.busy ? root.t("Reading and measuring application files… ", "قراءة ملفات التطبيقات وقياسها… ")+ApplicationStorage.scannedFiles : ApplicationStorage.summary.canceled ? root.t("Canceled. These are partial results.", "أُلغي الفحص. هذه نتائج جزئية.") : (ApplicationStorage.summary.capped || ApplicationStorage.summary.inventoryLimited) ? root.t("Scan limit reached. These are partial results.", "تم بلوغ حد الفحص. هذه نتائج جزئية.") : root.t("Measurement complete: ", "اكتمل القياس: ")+(ApplicationStorage.summary.scannedFiles || 0)+root.t(" unique files.", " ملف فريد.") }
 Note { visible: ApplicationStorage.busy && Object.keys(ApplicationStorage.summary).length>0; text: root.t("Previous results remain visible until this scan finishes.", "تبقى النتائج السابقة ظاهرة حتى انتهاء هذا الفحص.") }
 Note { visible: ApplicationStorage.error.length>0; text: ApplicationStorage.error; color: Prefs.dark ? "#ffaaa4" : "#b22b24" }
 Card {
  visible: Object.keys(ApplicationStorage.summary).length>0
  HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; font.bold: true; text: root.t("Measured total: ", "الإجمالي المقاس: ")+root.size(ApplicationStorage.summary.measuredBytes) }
  Note { text: root.t("Shared files, counted once: ", "الملفات المشتركة، محسوبة مرة واحدة: ")+root.size(ApplicationStorage.summary.sharedBytes) }
  Note { text: root.t("Unattributed data and settings: ", "بيانات وإعدادات غير منسوبة: ")+root.size(ApplicationStorage.summary.unattributedDataBytes) }
  Note { text: root.t("Unattributed cache: ", "ذاكرة مؤقتة غير منسوبة: ")+root.size(ApplicationStorage.summary.unattributedCacheBytes) }
  Note { text: root.t("Excluded links, unavailable paths, or unreadable entries: ", "روابط أو مسارات غير متاحة أو عناصر غير قابلة للقراءة مستثناة: ")+(ApplicationStorage.summary.skipped || 0) }
 }
 Note { text: root.t("Installed files come from the package owning each desktop application or its Flatpak deployment. Packages used by several apps and hard-linked files are shown separately as shared. Dependencies without a desktop app and Flatpak runtimes are excluded. Unattributed totals cover standard XDG folders, not your whole home. Directory metadata and symlinks are excluded; reflinks and compression can make physical disk usage differ. These figures are not a reclaimable-space estimate.", "تأتي الملفات المثبتة من الحزمة المالكة لملف التطبيق أو من تثبيت Flatpak. تُعرض الحزم التي تستخدمها تطبيقات متعددة والملفات ذات الروابط الصلبة منفصلة كمساحة مشتركة. تُستثنى التبعيات بلا تطبيق سطح مكتب وبيئات تشغيل Flatpak. تغطي المجاميع غير المنسوبة مجلدات XDG القياسية، وليس مجلدك الشخصي كله. تُستثنى بيانات المجلدات والروابط الرمزية؛ وقد تجعل الروابط المرجعية والضغط الاستخدام الفعلي مختلفًا. هذه الأرقام ليست تقديرًا لمساحة قابلة للاستعادة.") }
 HarborField { id: filter; Layout.fillWidth: true; visible: ApplicationStorage.applications.length>0; placeholderText: root.t("Find an application", "البحث عن تطبيق"); Accessible.name: placeholderText; onTextEdited: root.shown=20 }
 Repeater {
  model: root.matching.slice(0,root.shown)
  delegate: Card {
   id: app
   required property var modelData
   HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; textFormat: Text.PlainText; font.bold: true; text: app.modelData.name }
   Note { text: root.t("Attributed total: ", "الإجمالي المنسوب: ")+root.size(app.modelData.totalBytes) }
   Note { text: root.t("Installed files: ", "الملفات المثبتة: ")+(app.modelData.installedKnown ? root.size(app.modelData.installedBytes) : root.t("Unknown — package ownership was not found", "غير معروف — لم تُحدد الحزمة المالكة")) }
   Note { text: root.t("Data and settings: ", "البيانات والإعدادات: ")+(app.modelData.dataKnown ? root.size(app.modelData.dataBytes) : root.t("Not attributed; associate a folder below", "غير منسوبة؛ اربط مجلدًا أدناه")) }
   Note { text: root.t("Cache: ", "الذاكرة المؤقتة: ")+(app.modelData.cacheKnown ? root.size(app.modelData.cacheBytes) : root.t("Not attributed; associate a folder below", "غير منسوبة؛ اربط مجلدًا أدناه")) }
   Note { visible: app.modelData.sharedReferencedBytes>0; text: root.t("References shared files: ", "يشير إلى ملفات مشتركة: ")+root.size(app.modelData.sharedReferencedBytes)+root.t(" (excluded from this app’s total)", " (مستثناة من إجمالي هذا التطبيق)") }
  }
 }
 HarborButton { visible: root.matching.length>root.shown; text: root.t("Show more applications", "عرض تطبيقات إضافية"); onClicked: root.shown+=20 }
 Card {
  visible: ApplicationStorage.applications.length>0
  HarborLabel { Layout.fillWidth: true; wrapMode: Text.WordWrap; font.bold: true; text: root.t("Associate a native app folder", "ربط مجلد بتطبيق") }
  Note { text: root.t("Native applications do not reliably identify their data folders. Choose a folder inside your home directory only when you know it belongs to the selected app. Flatpak data, settings, and cache are mapped automatically.", "لا تحدد التطبيقات العادية مجلدات بياناتها بشكل موثوق. اختر مجلدًا داخل مجلدك الشخصي فقط عندما تعرف أنه يخص التطبيق المحدد. تُربط بيانات Flatpak وإعداداته وذاكرته المؤقتة تلقائيًا.") }
  ComboBox { id: choice; Layout.fillWidth: true; model: ApplicationStorage.applications; textRole: "name"; valueRole: "id"; Accessible.name: root.t("Application", "التطبيق") }
  ComboBox { id: kind; Layout.fillWidth: true; model: [root.t("Data and settings", "البيانات والإعدادات"),root.t("Cache", "الذاكرة المؤقتة")]; Accessible.name: root.t("Folder type", "نوع المجلد") }
  HarborField { id: folder; Layout.fillWidth: true; placeholderText: root.t("Absolute folder path inside your home", "مسار المجلد الكامل داخل مجلدك الشخصي"); Accessible.name: placeholderText }
  HarborButton { text: root.t("Review association", "مراجعة الربط"); enabled: !ApplicationStorage.busy&&choice.currentIndex>=0&&folder.text.length>0; onClicked: root.pendingAssociation={id:choice.currentValue,name:choice.currentText,path:folder.text,kind:kind.currentIndex===0?"data":"cache"} }
  ColumnLayout {
   Layout.fillWidth: true; visible: !!root.pendingAssociation.id
   Note { text: root.t("Associate this folder and its regular files with ", "هل تريد ربط هذا المجلد وملفاته العادية بالتطبيق ")+(root.pendingAssociation.name || "")+"?\n"+(root.pendingAssociation.path || "") }
   Flow {
    Layout.fillWidth: true; spacing: 8
    HarborButton { text: root.t("Confirm and measure", "تأكيد وقياس"); enabled: !ApplicationStorage.busy; onClicked: {ApplicationStorage.associateFolder(root.pendingAssociation.id,root.pendingAssociation.path,root.pendingAssociation.kind);root.pendingAssociation={}} }
    HarborButton { text: root.t("Cancel", "إلغاء"); onClicked: root.pendingAssociation={} }
   }
  }
 }
 Repeater {
  model: ApplicationStorage.associations
  delegate: ColumnLayout {
   id: association
   required property var modelData
   Layout.fillWidth: true
   Note { text: (ApplicationStorage.applications.find(function(app){return app.id===association.modelData.id}) || ({name:association.modelData.id})).name+" · "+(association.modelData.kind==="cache"?root.t("Cache","الذاكرة المؤقتة"):root.t("Data and settings","البيانات والإعدادات"))+"\n"+association.modelData.path; wrapMode: Text.WrapAnywhere }
   HarborButton { text: root.t("Remove association", "إزالة الربط"); enabled: !ApplicationStorage.busy; onClicked: ApplicationStorage.removeAssociation(association.modelData.id,association.modelData.path) }
  }
 }
}
