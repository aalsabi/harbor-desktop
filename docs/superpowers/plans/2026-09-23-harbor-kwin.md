# Harbor KWin Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** بناء جلسة Debian قابلة للتثبيت تستخدم KWin وواجهة Harbor المخصصة بالكامل.

**Architecture:** KWin مدير العرض والنوافذ، Qt6/QML لواجهة shell والإعدادات. خدمات Debian عبر محولات غير متزامنة مع حالة قدرة وفشل صريحة. لا تشغيل Plasma Shell أو استخدام مكوناته المرئية.

**Tech Stack:** C++20، CMake، Qt6 Quick/DBus/Test، KDE Frameworks 6، LayerShellQt، KScreen، KWin Wayland، NetworkManager، PipeWire، BlueZ، Polkit.

**Spec:** ../specs/2026-09-23-harbor-design.md

## Global Constraints

- الهدف Debian 13 amd64، Qt 6.8 وKWin 6.3 كأساس توافق.
- لا تشغيل لـplasmashell ولا استخدام panels أو widgets أو session startup الخاصة بـPlasma.
- Hyprland وlabwc خارج نطاق التنفيذ.
- كل وظيفة غير مكتملة تسجل كفجوة، ولا يستعاض عنها بزر يوحي بأنها تعمل.
- لا تضمين لملفات Apple في الحزمة.
- كل صف في التقرير يصنف: نجح، فشل، لم ينفذ، محاكاة فقط.

## Review Focus

- إعادة توصيل الشاشة: لا تبقى لوحة خارج الشاشة ولا مساحة محجوزة وهمية؛ مهمة 2.
- انهيار shell أثناء القفل: لا يكشف الجلسة ولا ينهي خدمة القفل؛ مهمة 1.
- اختفاء خدمة D-Bus أثناء الطلب: إنهاء busy وعرض فشل دون نجاح كاذب؛ مهمة 5.
- ملف desktop ضار أو معرف تطبيق ملتبس: لا تنفيذ shell ولا تركيز نافذة تطبيق آخر؛ مهمة 3.
- وجود جلسة KDE أخرى للمستخدم: لا الكتابة فوق إعداداتها أو إنهاء خدماتها؛ مهمة 1 و8.

## تقسيم التنفيذ

ثلاث حزم عمل مترابطة: A جلسة وshell (1–4)، B إعدادات وتكاملات (5–6)، C توزيع واعتماد (7–8). كل حزمة تنتج برنامجاً قابلاً للتجربة، لكن لا توصف الحزمة A وحدها بأنها المشروع الكامل. البدء باختبار تكامل KWin يقلل خطر بناء dock على بروتوكول لا يستطيع العميل استخدامه.

## 1. جلسة KWin معزولة وقابلة للفحص

**Files:** CMakeLists.txt، src/session/main.cpp، session/harbor.desktop، scripts/probe-kwin.sh، tests/integration/session_test.py، docs/design/kwin-contract.md.

**Interfaces:** `harbor-session --nested` ينشئ بيئة اختبار منفصلة؛ `harbor-session` يبدأ جلسة تسجيل دخول مستقلة. `harbor-shell --diagnose` يخرج JSON: `{"compositor":"KWin","windowManagement":true,"layerShell":true}` وفق القدرات الحقيقية، لا قيم ثابتة.

- [ ] فحص حزم التطوير ونسخ بروتوكولات KWin وKScreen وLayerShellQt على Debian 13؛ تسجيل واجهات البناء والـglobals الفعلية في kwin-contract.md قبل ربط النماذج بها.
- [ ] إنشاء اختبار يبدأ جلسة متداخلة بملف إعداد مؤقت، ينتظر socket، ثم يتحقق من عدم تشغيل Plasma ومن عدم تعديل إعداد المستخدم:

```python
assert session.wait_for_wayland(timeout=15)
assert 'plasmashell' not in session.child_process_names()
assert before_user_config == snapshot_user_config()
session.stop()
assert session.remaining_children() == []
```

- [ ] تشغيل الاختبار قبل التنفيذ وتسجيل الفشل؛ إنشاء مشرف جلسة يملك عملياته فقط، يمرر الإشارات ويحدد مهلة إنهاء، ويعزل KWin config في الاختبار. لا killall ولا pkill عام.
- [ ] اختبار البروتوكولات المقيدة للوصول للنوافذ، تسجيل آلية الثقة المدعومة فعلياً دون تسمية العملية plasmashell. إذا تعذر الوصول يوقف اعتماد dock الكامل ويعالج التكامل قبل مهمة 3.
- [ ] اختبار locker مفتوح متوافق مع النسخة المستهدفة؛ قتل shell أثناء القفل يجب ألا يزيل القفل. تشغيل خدمة القفل منفصلة؛ لا تضمين قفل مرئي تجريبي ضمن جلسة الإنتاج.
- [ ] تشغيل الاختبارات وإدراج السجل؛ حفظ تغيير مستقل في Git إذا أنشئ مستودع قابل للكتابة.

## 2. سطح المكتب والشريط والـdock بصرياً

**Files:** src/shell/main.cpp، src/shell/SurfaceManager.{h,cpp}، qml/shell/{Desktop,MenuBar,Dock,ControlCenter}.qml، qml/components/{Tokens,GlassCard,IconButton}.qml، tests/qml/tst_surfaces.qml، tests/integration/screens_test.py.

**Interfaces:** `SurfaceManager::syncScreens()`؛ QML properties: `reduceMotion: bool`, `glassOpacity: real`, `accent: color`. الربط مع LayerShellQt يحدد الشاشة والحافة والطبقة والمساحة المحجوزة لكل سطح.

- [ ] اختبار تفاعل حقيقي لتركيز لوحة المفاتيح وEscape:

```qml
function test_closeControlCenter() {
    panel.open()
    verify(panel.visible)
    keyClick(Qt.Key_Escape)
    tryCompare(panel, "visible", false)
}
```

- [ ] إثبات الفشل، ثم بناء سطح لكل شاشة؛ الخلفية في طبقة خلفية، الشريط في الأعلى، dock مع مساحة حجز محددة؛ لا تمديد نافذة شفافة كبيرة تعترض النقرات.
- [ ] بناء الرموز والخلفية الأصلية وdesign tokens؛ دعم reduced motion وRTL وتكبير العرض. تسجيل منشأ الأصول وترخيصها عند إنشائها.
- [ ] اختبار فصل شاشة وإعادتها ثلاث مرات: عدد أسطح الشريط يساوي عدد الشاشات، ولا تظهر أسطح قديمة. تحقق بصري عند 100/150/200% ووضع معتم.
- [ ] فحص Qt accessibility names ومسار Tab ثم حفظ النتائج والتغيير.

## 3. التطبيقات والنوافذ والقوائم والبحث

**Files:** src/apps/{ApplicationCatalog,Launcher,DockModel}.{h,cpp}، src/kwin/WindowModel.{h,cpp}، src/menu/{GlobalMenu,StatusNotifierHost}.{h,cpp}، qml/shell/Launcher.qml، tests/unit/apps_test.cpp، tests/integration/windows_test.py.

**Interfaces:** `Launcher::launch(QString desktopId)`؛ `WindowModel::activate(QString id)` و`close(QString id)`؛ DockModel roles: desktopId, title, icon, windowIds, pinned. استعمال KDE KService/KIO لإطلاق desktop entries دون shell، وتوثيق APIs المتاحة في إصدار البناء.

- [ ] كتابة fixtures لتطبيق Hidden وNoDisplay واسم يحوي مسافات وExec يحوي رموز shell؛ اختبار أن العلامة لا تنشأ:

```cpp
QVERIFY(!catalog.visibleIds().contains("hidden.desktop"));
launcher.launch("literal-argument.desktop");
QVERIFY(!QFile::exists(markerPath));
```

- [ ] إثبات الفشل ثم تنفيذ قراءة XDG ومراقبة التغييرات والإطلاق الآمن؛ لا parsing يدوي مبسط لسطر Exec.
- [ ] وصل نوافذ KWin بمعرفات سطح المكتب مع معالجة غياب appId: النافذة المجهولة تظهر مستقلة ولا تنسب تعسفياً لتطبيق مثبت.
- [ ] إضافة تثبيت وإزالة وتركيز نافذة وقائمة اختيار عدة نوافذ وبحث التطبيقات.
- [ ] تنفيذ DBusMenu للتطبيق المصدر وStatusNotifierItem؛ تطبيق غير مصدر لا يحصل على قائمة صورية. اختبار خروج التطبيق أثناء فتح قائمته وإعادة تسجيل tray بعد انهياره.
- [ ] اختبار تطبيقين Qt وGTK ونافذتين للتطبيق الواحد؛ تثبيت دليل فعلي للتركيز والإغلاق وحفظ التغيير.

## 4. الإشعارات والاختصارات والزخارف

**Files:** src/notifications/NotificationModel.{h,cpp}، qml/shell/NotificationCenter.qml، config/kwin/، src/session/Shortcuts.{h,cpp}، tests/integration/notifications_test.py، docs/design/decorations.md.

**Interfaces:** خدمة واحدة `org.freedesktop.Notifications`؛ النموذج يعرض id, summary, body, actions, expiresAt. اختيار خدمة مفتوحة توفر تاريخاً وأحداثاً موثقة، أو بناء الخدمة ضمن Harbor إذا لم توجد واجهة مناسبة؛ لا ادعاء وجود history لدى daemon لا يوفره.

- [ ] اختبار ReplaceId وCloseNotification وActionInvoked باستخدام عميل D-Bus؛ sanitize المحتوى المعروض ولا تحميل صور شبكية من الإشعارات.
- [ ] ربط مركز الإشعارات بالخدمة المختارة واختبار عدم انتزاع اسم D-Bus من خدمة قائمة.
- [ ] إعداد اختصارات البحث ومركز التحكم والتنقل عبر KWin/GlobalAccel المدعوم، مع كشف التعارض وتسجيل إعدادات Harbor فقط.
- [ ] إعداد زخرفة أصلية عبر واجهة KDecoration المتوافقة أو ثيم محرك مفتوح متاح؛ اختبار أزرار الإغلاق والتصغير والتكبير فعلياً. تسجيل التطبيقات ذات client decorations كفجوة منفصلة.
- [ ] تسجيل نتائج الاختبار وحفظ التغيير.

## 5. محولات خدمات Debian

**Files:** src/services/{Operation,Network,Audio,Bluetooth,Power,Accounts,Updates}.{h,cpp}، tests/services/{network,audio,bluetooth,power,accounts,updates}_test.cpp.

**Interfaces:** `Operation` QObject properties: busy, error, available؛ signal `finished(bool ok, QString error)`؛ كل خدمة تعرض حالة مقروءة وتعيد `Operation*` من إجراء التغيير. `setWifiEnabled(bool)`، `setVolume(double)`، `setMuted(bool)`، `setBluetoothEnabled(bool)`؛ مجال volume من 0 إلى 1. الحسابات والتحديثات عبر D-Bus وتفويض Polkit دون وسيط root عام.

- [ ] كتابة اختبار إسقاط الخدمة أثناء الطلب:

```cpp
Operation *op = network.setWifiEnabled(true);
fakeNetwork.disconnectBus();
QTRY_VERIFY_WITH_TIMEOUT(!op->busy(), 6000);
QVERIFY(!op->error().isEmpty());
```

- [ ] إثبات الفشل ثم تنفيذ الطلبات غير المتزامنة، مهلة 5 ثوان لطلبات الضبط العادية؛ معاملات التحديث ذات تقدم وإلغاء مستقل لا تطبق عليها مهلة ضبط قصيرة.
- [ ] الشبكة: عرض الأجهزة والشبكات والاتصال والمحفوظة؛ الأسرار لا تكتب في config أو stdout. Bluetooth: scan وpair وconnect مع agent مدعوم.
- [ ] الصوت: قياس وضبط جهاز الإخراج الافتراضي والكتم والمستوى؛ اختيار مخرجات متاحة. الطاقة: UPower وpower profiles/backlight مع عرض عدم الدعم.
- [ ] AccountsService لإدارة المستخدمين بتفويض النظام؛ PackageKit للتحديثات أو إحالة مسماة بوضوح عند عدم توفره. لا تعرض الإحالة كواجهة داخلية مكتملة.
- [ ] اختبارات رفض الصلاحية والمدخل غير الصالح وعودة الخدمة. الاختبارات الفعلية على عتاد/session منفصلة عن mocks في التقرير.

## 6. تطبيق الإعدادات والعرض

**Files:** src/settings/main.cpp، src/settings/Preferences.{h,cpp}، src/display/DisplayTransaction.{h,cpp}، qml/settings/{Main,Network,Audio,Bluetooth,Display,Appearance,Users,Updates,Input,Power}.qml، tests/unit/preferences_test.cpp، tests/integration/display_test.py.

**Interfaces:** `Preferences::setValue(QString key,QVariant value)` مع schema وقيم افتراضية؛ `DisplayTransaction::apply(QVariantMap config)` و`confirm()` و`revert()`، timer 15 ثانية، KScreen خلفية التنفيذ.

- [ ] اختبار الرجوع بفعل انتهاء المهلة:

```python
original = displays.snapshot()
transaction.apply(valid_alternative)
transaction.advance_test_clock(15001)
assert displays.snapshot() == original
```

- [ ] إثبات الفشل؛ تنفيذ نسخة مؤقتة من إعداد العرض وتأكيد ثم حفظ، والرجوع عند فشل التطبيق/انتهاء المهلة. فصل شاشة أثناء المعاملة يعيد حساب التكوين الآمن.
- [ ] تنفيذ صفحات الإعدادات فوق خدمات مهمة 5، مع بحث الأقسام ومؤشرات busy ورسائل قابلة للفهم وعدم حفظ نجاح متفائل.
- [ ] ربط المظهر ومركز التحكم بنفس Preferences والخدمات؛ اختبار تبديل الصوت في أحدهما ينعكس على الآخر.
- [ ] تخطيط عربي/إنجليزي واختبارات استعادة config معطل، وإدخال/مؤشر عبر APIs KWin المثبتة في عقد المهمة 1.
- [ ] حفظ نتائج الاختبار والتغيير.

## 7. التثبيت والتوثيق والأصول

**Files:** packaging/debian/{control,rules,changelog,copyright,install}، scripts/{build,package,doctor}.sh، LICENSE، THIRD_PARTY_NOTICES.md، docs/{INSTALL,USAGE,DESIGN,GAPS,SOURCES}.md.

- [ ] بناء حزمة في بيئة Debian 13 نظيفة؛ توليد Depends عبر debhelper/shlibs وعدم الاعتماد على حزمة مبنية على sid باسم trixie.
- [ ] اختبار تثبيت وإزالة مع ملفات مستخدم موجودة مسبقاً:

```python
assert hash_file(user_kwinrc) == original_hash
assert installed_session.exec_name == 'harbor-session'
assert not package_starts_plasmashell()
```

- [ ] تضمين جلسة جديدة دون تغيير جلسة الدخول الافتراضية؛ لا postinst يكتب HOME ولا تحديث تلقائي لمستودعات المستخدم.
- [ ] كتابة تعليمات البناء والتثبيت وتشخيص غياب الخدمات والإزالة والرجوع؛ ترخيص كل أصل ومصدر اعتمادية وتوضيح حدود GTK4/XWayland.
- [ ] فحص حزمة deb ومراجعة مساراتها واعتمادياتها، ثم إنتاج أرشيف المصدر وSHA256SUMS.

## 8. الاعتماد والتقرير النهائي

**Files:** scripts/benchmark.py، tests/compatibility/matrix.json، docs/{PERFORMANCE,COMPATIBILITY}.md، evidence/.

- [ ] تشغيل build وctest وQML tests وفحص الحزمة مرة على الإصدار المستهدف، وحفظ الأوامر والنسخ والنتائج.
- [ ] تشغيل مصفوفة Qt/GTK3/GTK4/Electron/XWayland، شاشتين، قياس كسري، العربية، portals ومشاركة الشاشة، قفل وتعليق، وخروج نظيف؛ وضع «لم ينفذ» لكل حالة غير متاحة.
- [ ] قياس RSS وCPU بعد 60 ثانية استقرار، 30 عينة فتح لوحة، وframe times؛ كتابة JSON خام مع CPU/GPU والنسخ وrenderer. لا مقارنة headless بالعتاد.
- [ ] مراجعة المصدر واعتمادياته للتأكد من عدم تشغيل Plasma Shell أو العبث بجلسة قائمة؛ اختبار إعادة تشغيل Harbor shell وحده.
- [ ] مراجعة فجوات المواصفة بنداً بنداً، وفصل منفذ ومختبر ومنفذ غير مختبر وغير منفذ، ثم تسليم الحزمة والمصادر والتقارير.

## حالة الخطة

هذه خطة وليست نتائج تنفيذ. لا توجد حزمة مبنية أو اعتماد لـKWin في البيئة الحالية بعد. مرحلة التحقق الأولى مطالبة بتثبيت عقد تكامل KWin عملياً قبل بناء المكونات المعتمدة عليه. إذا تعذر اختبار الإصدار المستهدف يبقى التسليم تجريبياً مع السبب الدقيق، ولا يتحول ذلك إلى ادعاء توافق.

التنفيذ المقترح: مباشر في هذه المهمة مع مراجعة مستقلة عند نهاية التغييرات، لأن نماذج النوافذ والخدمات والواجهة مترابطة. البديل تنفيذ بمساعدين لكل مكون ومراجعة لكل مهمة.
