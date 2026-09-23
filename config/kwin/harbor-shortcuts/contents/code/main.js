function show(page) { callDBus("org.harbor.Shell", "/Shell", "org.harbor.Shell", "Show", page); }
registerShortcut("HarborSearch", "Harbor: search applications", "Meta+Space", function () { show("launcher"); });
registerShortcut("HarborControl", "Harbor: control center", "Meta+Shift+C", function () { show("control"); });
registerShortcut("HarborSettings", "Harbor: system settings", "Meta+,", function () { show("settings"); });
registerShortcut("HarborNotifications", "Harbor: notifications", "Meta+N", function () { show("notifications"); });
