# المصادر المرجعية

تاريخ المراجعة: 2026-09-23. الروابط مرجع للتصميم والتكامل، وليست ترخيصاً لنسخ أصولها.

- Apple، macOS Golden Gate27 والتغييرات المرئية: https://www.apple.com/os/macos/
- Apple، معلومات Golden Gate: https://support.apple.com/en-us/148830
- Apple، شريط القوائم: https://support.apple.com/en-my/guide/mac-help/mchlp1446/mac
- Debian13 trixie: https://www.debian.org/releases/trixie/
- حزمة KWin: https://packages.debian.org/trixie/kwin-wayland
- Qt6 في Debian13: https://packages.debian.org/trixie/qt6-base-dev
- KWin6.3.6، تحميل kwinrc: https://sources.debian.org/src/kwin/4:6.3.6-1/src/main.cpp/
- KWin، فحص البروتوكولات وscreen locker: https://sources.debian.org/src/kwin/4:6.3.6-1/src/wayland_server.cpp/
- LayerShellQt: https://invent.kde.org/plasma/layer-shell-qt
- KWayland: https://invent.kde.org/plasma/kwayland
- KWin: https://invent.kde.org/plasma/kwin
- Qt Quick: https://doc.qt.io/qt-6/qtquick-index.html
- GIO desktop application launch: https://docs.gtk.org/gio-unix/class.DesktopAppInfo.html
- Desktop entries: https://specifications.freedesktop.org/desktop-entry-spec/latest/
- Notifications: https://specifications.freedesktop.org/notification-spec/latest/
- StatusNotifierItem: https://www.freedesktop.org/wiki/Specifications/StatusNotifierItem/
- NetworkManager nmcli: https://networkmanager.dev/docs/api/latest/nmcli.html
- WirePlumber wpctl: https://pipewire.pages.freedesktop.org/wireplumber/tools/wpctl.html

استنتاج التصميم: استخدام KWin مع shell مستقل يحقق تفضيل المستخدم لـKDE دون تشغيل Plasma Shell. البروتوكولات وبعض المكتبات تقع ضمن مشاريع KDE Plasma، لكن لا تُستعمل واجهة plasmashell. لا يوجد إثبات لتطابق بصري أو وظيفي100%، ولا يُدّعى ذلك.
