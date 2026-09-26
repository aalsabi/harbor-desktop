import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout{
 id:root;objectName:"printersPage";spacing:14
 property bool initialDiscovery:true
 property string removeTarget:""
 property string cancelTarget:""
 property color ink:Prefs.dark?"#eeeef0":"#252527"
 property color muted:Prefs.dark?"#a6a6ad":"#76767c"
 function t(en,ar){return Prefs.language==="ar"?ar:en}
 component Choice:ComboBox{Layout.fillWidth:true;contentItem:Text{text:parent.displayText;color:root.ink;verticalAlignment:Text.AlignVCenter;leftPadding:10;rightPadding:24;elide:Text.ElideRight}background:Rectangle{radius:6;color:Prefs.dark?"#45454b":"#fafafa";border.color:Prefs.dark?"#55555b":"#d9d9df"}}
 Component.onCompleted:Printers.refresh()
 Connections{target:Printers;function onChanged(){if(root.initialDiscovery&&!Printers.busy){root.initialDiscovery=false;Printers.discover()}}}
 Connections{target:PrinterDrivers;function onInstalled(){if(!Printers.busy)Printers.loadDrivers()}}
 RowLayout{Layout.fillWidth:true
  Text{Layout.fillWidth:true;wrapMode:Text.WordWrap;color:root.muted;font.pixelSize:13;text:root.t("Printers and pending jobs from your printing service.","الطابعات والمهام المعلقة من خدمة الطباعة.")}
  HarborButton{text:Printers.busy?root.t("Loading…","جارٍ التحميل…"):root.t("Refresh","تحديث");enabled:!Printers.busy;onClicked:Printers.refresh()}
 }
 Rectangle{Layout.fillWidth:true;implicitHeight:notice.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7"
  Text{id:notice;anchors.fill:parent;anchors.margins:14;wrapMode:Text.WordWrap;color:root.muted;font.pixelSize:13;text:root.t("Add an IPP printer or manage existing queues. Administrator changes require system authorization. Printing preferences apply to your account.","أضف طابعة IPP أو أدر قوائم الانتظار الحالية. تتطلب تغييرات المسؤول مصادقة النظام. تنطبق تفضيلات الطباعة على حسابك.")}
 }
 Text{Layout.fillWidth:true;visible:!Printers.available&&!Printers.busy;wrapMode:Text.WordWrap;color:root.ink;font.pixelSize:13;text:root.t("Printing tools are unavailable. Install cups-client to use printer settings.","أدوات الطباعة غير متاحة. ثبّت cups-client لاستخدام إعدادات الطابعات.")}
 Text{Layout.fillWidth:true;visible:Printers.available&&!Printers.busy&&Printers.devices.length===0;wrapMode:Text.WordWrap;color:root.muted;font.pixelSize:13;text:root.t("No configured printers were found. Check that the printing service is running and a printer is configured.","لم يتم العثور على طابعات مُعدّة. تحقق من تشغيل خدمة الطباعة وإعداد طابعة.")}
 Repeater{model:Printers.devices;delegate:Rectangle{
  required property var modelData
  Layout.fillWidth:true;implicitHeight:body.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7"
  ColumnLayout{id:body;anchors.fill:parent;anchors.margins:14;spacing:8
   RowLayout{Layout.fillWidth:true
    Text{Layout.fillWidth:true;text:modelData.name;elide:Text.ElideRight;color:root.ink;font.bold:true;font.pixelSize:14}
    HarborButton{text:Printers.defaultPrinter===modelData.name?root.t("Default","الافتراضية"):root.t("Set as default","تعيين كافتراضية");enabled:!Printers.busy&&Printers.defaultPrinter!==modelData.name;onClicked:Printers.setDefault(modelData.name)}
   }
   Text{Layout.fillWidth:true;text:modelData.status;wrapMode:Text.Wrap;color:root.muted;font.pixelSize:12}
   Text{text:root.t("Pending jobs: ","المهام المعلقة: ")+modelData.jobs.length;color:root.muted;font.pixelSize:12}
   RowLayout{HarborButton{text:root.t("Printing options","خيارات الطباعة");enabled:!Printers.busy;onClicked:Printers.loadOptions(modelData.name)}HarborButton{text:root.t("Remove printer…","إزالة الطابعة…");enabled:!Printers.busy;onClicked:root.removeTarget=modelData.name}}
   Repeater{model:modelData.jobs;delegate:RowLayout{required property var modelData;Layout.fillWidth:true;Text{Layout.fillWidth:true;text:modelData.id+" · "+modelData.owner+" · "+modelData.bytes+root.t(" bytes"," بايت");wrapMode:Text.Wrap;color:root.ink;font.pixelSize:12}HarborButton{text:root.t("Cancel job…","إلغاء المهمة…");enabled:!Printers.busy;onClicked:root.cancelTarget=modelData.id}}}
  }
 }}
 Rectangle{visible:root.removeTarget.length>0||root.cancelTarget.length>0;Layout.fillWidth:true;implicitHeight:confirmBody.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white"
  ColumnLayout{id:confirmBody;anchors.fill:parent;anchors.margins:14
   Text{Layout.fillWidth:true;wrapMode:Text.Wrap;color:root.ink;text:root.removeTarget?root.t("Permanently remove this printer queue? ","إزالة قائمة انتظار هذه الطابعة نهائيًا؟ ")+root.removeTarget:root.t("Cancel this print job? ","إلغاء مهمة الطباعة هذه؟ ")+root.cancelTarget}
   RowLayout{HarborButton{text:root.t("Keep","إبقاء");onClicked:{root.removeTarget="";root.cancelTarget=""}}HarborButton{text:root.t("Confirm","تأكيد");enabled:!Printers.busy;onClicked:{if(root.removeTarget)Printers.removePrinter(root.removeTarget,true);else Printers.cancelJob(root.cancelTarget,true);root.removeTarget="";root.cancelTarget=""}}}
  }
 }
 Rectangle{visible:Printers.selectedPrinter.length>0;Layout.fillWidth:true;implicitHeight:optionsBody.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white"
  ColumnLayout{id:optionsBody;anchors.fill:parent;anchors.margins:14;spacing:8
   Text{Layout.fillWidth:true;color:root.ink;wrapMode:Text.Wrap;text:root.t("Personal printing options — ","خيارات الطباعة لحسابك — ")+Printers.selectedPrinter}
   Repeater{model:Printers.options;delegate:ColumnLayout{required property var modelData;Layout.fillWidth:true
    Text{text:modelData.name;color:root.ink;font.pixelSize:13}
    Choice{model:modelData.choices;currentIndex:modelData.currentIndex;enabled:!Printers.busy;onActivated:{Printers.setOption(Printers.selectedPrinter,modelData.key,currentText);currentIndex=Qt.binding(function(){return modelData.currentIndex})}}
   }}
   Text{visible:!Printers.busy&&Printers.options.length===0;Layout.fillWidth:true;wrapMode:Text.Wrap;color:root.muted;text:root.t("The printer did not report configurable options.","لم تُرجع الطابعة خيارات قابلة للتعديل.")}
  }
 }
 Rectangle{Layout.fillWidth:true;implicitHeight:addBody.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7"
  ColumnLayout{id:addBody;anchors.fill:parent;anchors.margins:14;spacing:10
   Text{text:root.t("Add printer","إضافة طابعة");color:root.ink;font.bold:true;font.pixelSize:14}
   RowLayout{Layout.fillWidth:true
    Text{Layout.fillWidth:true;wrapMode:Text.Wrap;color:root.muted;text:root.t("Turn on your printer and connect it to this computer or the same network.","شغّل الطابعة ووصلها بهذا الحاسوب أو بالشبكة نفسها.")}
    HarborButton{text:root.t("Find printers","البحث عن طابعات");enabled:!Printers.busy;onClicked:Printers.discover()}
   }
   Repeater{model:Printers.discovered;delegate:RowLayout{required property var modelData;Layout.fillWidth:true
    ColumnLayout{Layout.fillWidth:true;Text{Layout.fillWidth:true;text:modelData.name;color:root.ink;wrapMode:Text.Wrap}Text{Layout.fillWidth:true;text:modelData.uri;color:root.muted;font.pixelSize:12;wrapMode:Text.Wrap}}
    HarborButton{text:root.t("Select","اختيار");enabled:!Printers.busy;onClicked:{printerUri.text=modelData.uri;printerName.text=modelData.name.replace(/[^a-zA-Z0-9_-]/g,"_").substring(0,60)||"Printer";Printers.selectDevice(modelData.uri);family.currentIndex=Math.max(0,family.model.indexOf(PrinterDrivers.suggestedFamily(modelData.model||modelData.name)))}}
   }}
   Text{Layout.fillWidth:true;wrapMode:Text.Wrap;color:root.muted;text:root.t("Choose a discovered printer, or enter an IPP address below. Driverless printers usually need no extra download. Installed drivers are matched by the printer’s reported model ID when available.","اختر طابعة مكتشفة أو أدخل عنوان IPP أدناه. لا تحتاج الطابعات التي تعمل دون برنامج تشغيل عادةً إلى تنزيل إضافي. تتم مطابقة برامج التشغيل المثبتة بمعرّف الطراز عند توفره.")}
   HarborField{id:printerName;Layout.fillWidth:true;placeholderText:root.t("Printer name","اسم الطابعة");Accessible.name:placeholderText}
   HarborField{id:printerUri;Layout.fillWidth:true;placeholderText:"ipp://printer.local/ipp/print";Accessible.name:root.t("Printer IPP address","عنوان IPP للطابعة")}
   Choice{id:driver;model:Printers.drivers;textRole:"name";valueRole:"id";Accessible.name:root.t("Printer driver","برنامج تشغيل الطابعة")}
   RowLayout{HarborButton{text:root.t("Load installed drivers","تحميل برامج التشغيل المثبتة");enabled:!Printers.busy;onClicked:Printers.loadDrivers()}HarborButton{text:root.t("Add printer","إضافة طابعة");prominent:true;enabled:!Printers.busy&&printerName.text.length>0&&printerUri.text.length>0&&driver.currentIndex>=0;onClicked:Printers.addPrinter(printerName.text,printerUri.text,driver.currentValue)}}
  }
 }
 Rectangle{Layout.fillWidth:true;implicitHeight:driverBody.implicitHeight+28;radius:10;color:Prefs.dark?"#303034":"white";border.color:Prefs.dark?"#454549":"#e2e2e7"
  ColumnLayout{id:driverBody;anchors.fill:parent;anchors.margins:14;spacing:10
   Text{text:root.t("Missing printer driver?","برنامج تشغيل الطابعة مفقود؟");color:root.ink;font.bold:true}
   Text{Layout.fillWidth:true;wrapMode:Text.Wrap;color:root.muted;text:root.t("Find drivers from your distribution’s enabled repositories. Family suggestions are not a guarantee of model compatibility. Review the package description and every dependency change before installing. Some printers need drivers not offered here.","ابحث عن برامج التشغيل في مستودعات التوزيعة المفعّلة. اقتراح الشركة لا يضمن توافق الطراز. راجع وصف الحزمة وكل تغيير في التبعيات قبل التثبيت. تحتاج بعض الطابعات إلى برامج غير متاحة هنا.")}
   RowLayout{Layout.fillWidth:true
    Choice{id:family;model:["HP","Epson","Brother","Samsung","Other"];enabled:!PrinterDrivers.busy;Accessible.name:root.t("Printer family","الشركة المصنعة للطابعة")}
    HarborButton{text:root.t("Find missing drivers","البحث عن برامج التشغيل المفقودة");enabled:!PrinterDrivers.busy;onClicked:PrinterDrivers.search(family.currentText)}
   }
   Repeater{model:PrinterDrivers.packages;delegate:RowLayout{required property var modelData;Layout.fillWidth:true
    Text{Layout.fillWidth:true;wrapMode:Text.Wrap;color:root.ink;text:modelData.name+" · "+modelData.version+"\n"+modelData.summary}
    HarborButton{text:root.t("Review install…","مراجعة التثبيت…");enabled:!PrinterDrivers.busy;onClicked:PrinterDrivers.prepare([modelData.id])}
   }}
   Repeater{model:PrinterDrivers.preview;delegate:Text{required property var modelData;Layout.fillWidth:true;wrapMode:Text.Wrap;color:root.ink;text:modelData.action+" · "+modelData.name+" · "+modelData.version}}
   Text{visible:PrinterDrivers.readyToInstall;Layout.fillWidth:true;wrapMode:Text.Wrap;color:root.ink;text:root.t("Install these packages and apply all changes shown above? The system may ask for administrator authorization.","هل تريد تثبيت هذه الحزم وتطبيق كل التغييرات أعلاه؟ قد يطلب النظام مصادقة المسؤول.")}
   RowLayout{visible:PrinterDrivers.readyToInstall
    HarborButton{text:root.t("Cancel","إلغاء");enabled:!PrinterDrivers.busy;onClicked:PrinterDrivers.discardPreview()}
    HarborButton{text:root.t("Confirm and install","تأكيد وتثبيت");prominent:true;enabled:!PrinterDrivers.busy;onClicked:PrinterDrivers.install(true)}
   }
   ProgressBar{visible:PrinterDrivers.busy;Layout.fillWidth:true;indeterminate:PrinterDrivers.percentage<0;value:PrinterDrivers.percentage/100}
   Text{Layout.fillWidth:true;visible:text.length>0;wrapMode:Text.Wrap;color:root.muted;text:PrinterDrivers.status}
   Text{Layout.fillWidth:true;visible:text.length>0;wrapMode:Text.Wrap;color:root.ink;text:PrinterDrivers.error}
   Text{Layout.fillWidth:true;visible:text.length>0;wrapMode:Text.Wrap;color:root.ink;text:PrinterDrivers.restart}
  }
 }
 Text{Layout.fillWidth:true;visible:text.length>0;text:Printers.message;wrapMode:Text.Wrap;color:root.ink;font.pixelSize:13}
}
