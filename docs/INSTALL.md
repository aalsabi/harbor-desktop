# التثبيت والبناء

هذه نسخة تطوير تجريبية. افصل بين نجاح بناء Debian 13 وبين اعتماد جهازك؛ راجع COMPATIBILITY.md قبل استخدامها كجلسة يومية.

## تثبيت الحزمة

على Debian 13 amd64، من المجلد الذي يحوي الحزمة:

    sudo apt install ./harbor-desktop_0.3.0_amd64.deb

لا تستخدم dpkg وحده لتجاوز الاعتماديات. تسجيل الخروج ثم اختيار **Harbor** من مدير الدخول يبدأ الجلسة الجديدة. لا يتغير اختيار الجلسة الافتراضي تلقائياً.

تطبيق الإعدادات:

    harbor-settings

معاينة المكونات داخل نافذة:

    harbor-shell --preview

جلسة متداخلة داخل جلسة رسومية قائمة:

    harbor-session --nested

تستخدم الجلسة المتداخلة ناقل D-Bus وملفات إعداد وبيانات مؤقتة مستقلة. يجب أن يدعم المضيف Wayland أو X11 للتشغيل المتداخل. المسار headless مخصص للاختبار، وليس بديلاً عن قياس GPU.

## البناء من المصدر

ثبت اعتماديات البناء:

    sudo apt install build-essential cmake pkg-config qt6-base-dev qt6-declarative-dev qt6-svg-dev liblayershellqtinterface-dev libkf6windowsystem-dev kwayland-dev libglib2.0-dev dbus-x11 python3 file

ثم من جذر المصدر:

    ./scripts/build.sh
    ./scripts/package.sh

ملف scripts/Dockerfile.trixie يصف بيئة بناء Debian 13. مثال:

    docker build -t harbor-build:trixie -f scripts/Dockerfile.trixie .
    mkdir -p build-trixie
    docker run --rm -v "$PWD:/src:ro" -v "$PWD/build-trixie:/build" harbor-build:trixie

مصدر Debian trixie يستمر بتلقي تحديثات؛ سجل أرقام الاعتماديات مع كل بناء. هذا وصف لإعادة البناء، وليس ادعاء تطابق الأرشيف بتاً ببت.

## الإعدادات والخدمات

- تفضيلات Harbor: ~/.config/harbor/settings.ini أو المسار المكافئ تحت XDG_CONFIG_HOME.
- إعداد KWin لهذه الجلسة فقط: ~/.config/harbor/session/kwinrc.
- لا تعدّل kwinrc الخاص بجلسة KDE أخرى.
- الشبكة تحتاج NetworkManager؛ الأجهزة التي يديرها ifupdown لا تظهر تلقائياً كاتصالات قابلة للتحكم.
- الصوت يحتاج PipeWire/WirePlumber؛ Bluetooth يحتاج BlueZ.
- اختيار أجهزة الصوت والاقتران المتقدم والشبكات المحفوظة يفتح أدوات خارجية مسماة بوضوح.
- تشغيل polkit-kde-agent-1 يدعم طلبات الصلاحيات؛ لا تمنح الحزمة صلاحيات عامة دون كلمة مرور.
- إعدادات المستخدمين تعرض AccountsService وتتيح تغيير الاسم الكامل. إنشاء الحسابات وكلمات المرور غير منفذين.
- نافذة التحديثات تحيل إلى Discover عند تثبيته؛ ليست مدير تحديث داخلياً.
- قفل الشاشة يعتمد على خدمة القفل المتوافقة مع KWin في التثبيت المستهدف، ولم يعتمد أمنياً في هذا الإصدار. لا تعتمد هذه النسخة على أجهزة غير مراقبة قبل اختبار القفل.

## الإزالة والرجوع

اختر جلسة سطح المكتب السابقة من مدير الدخول. ثم:

    sudo apt remove harbor-desktop

تبقى إعدادات Harbor الخاصة بك. حذف ~/.config/harbor اختياري ويمس تفضيلات Harbor فقط. لا تزل KWin أو مكتبات KDE إن كانت تستخدمها جلسات أخرى.

## التشخيص

    harbor-doctor

إذا كانت إدارة النوافذ غير متاحة، تحقق من تثبيت org.harbor.Shell.desktop ومن تطابق Exec مع الملف التنفيذي. لا تعطل فحوص الصلاحيات في KWin. تأكد من عدم تشغيل نسختين من Harbor على ناقل الجلسة نفسه.

## إعادة اختبار الحزمة في حاوية معزولة

بعد بناء صورة Docker المذكورة، يمكن تشغيل scripts/validate-trixie.sh داخلها فقط، مع ربط المصدر في /src ومجلد بناء قابل للكتابة في /build. يفحص البناء والاختبارات والتثبيت والجلسة الافتراضية والإزالة. يحتاج وصولاً لمستودعات Debian. إزالة capability من KWin ضمن هذا المشغّل تخص نسخة الحاوية حصراً.

أثناء تثبيت سطح مكتب Debian مصغر جداً، ثبت x11-common وتحقق من تجهيز /tmp/.X11-unix بواسطة النظام إذا أردت تطبيقات XWayland. اختبار التسليم اعتمد تطبيقات Wayland فقط.

## مدير الملفات

    harbor-files

زر Files في dock يفتح Harbor Files بعد ترقية الحزمة وإعادة تشغيل جلسة Harbor. لا تحتاج تغيير مدير الملفات الافتراضي للنظام. للتجربة من المصدر دون تثبيت، أضف مجلد scripts إلى PATH عند تشغيل build/harbor-shell --files حتى يجد عامل عمليات الملفات؛ راجع docs/FILES.md.
