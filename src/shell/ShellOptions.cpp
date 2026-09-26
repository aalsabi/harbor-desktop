#include "ShellOptions.h"

namespace {
// Pages accepted by --settings; any other value leaves the default page.
const QStringList settingsPages{"Gestures",
                                "Notifications & Focus",
                                "Startup Applications",
                                "Privacy & Permissions",
                                "Date & Time",
                                "Storage",
                                "Default Applications",
                                "Mouse",
                                "Touchpad",
                                "Printers",
                                "Language & Region",
                                "General",
                                "Wi-Fi",
                                "Bluetooth",
                                "Network",
                                "Sound",
                                "Appearance",
                                "Accessibility",
                                "Desktop & Dock",
                                "Displays",
                                "Keyboard",
                                "Battery",
                                "Users & Groups",
                                "About",
                                "Software Update"};
} // namespace

ShellOptions ShellOptions::parse(const QStringList& args) {
    ShellOptions o;
    o.preview = args.contains("--preview");
    o.settings = args.contains("--settings");
    o.files = args.contains("--files");
    o.control = args.contains("--control");
    o.diagnose = args.contains("--diagnose");
    if (o.control) {
        auto requested = args.value(args.indexOf("--control") + 1);
        if (QStringList{"HarborMenu", "BluetoothMenu", "RecentItems"}.contains(requested))
            o.controlPage = requested;
    }
    if (o.files || o.preview) {
        int n = args.indexOf(o.files ? "--files" : "--preview");
        if (n + 1 < args.size() && !args[n + 1].startsWith("--"))
            o.initialPath = args[n + 1];
    }
    if (o.settings) {
        int n = args.indexOf("--settings") + 1;
        if (n < args.size() && !args[n].startsWith("--") && settingsPages.contains(args[n]))
            o.settingsPage = args[n];
    }
    int i = args.indexOf("--screenshot");
    if (i >= 0 && i + 1 < args.size()) {
        o.screenshot = true;
        o.screenshotPath = args[i + 1];
        int s = args.indexOf("--screenshot-size");
        if (s >= 0 && s + 1 < args.size()) {
            auto parts = args[s + 1].split('x');
            if (parts.size() == 2) {
                int w = parts[0].toInt(), h = parts[1].toInt();
                if (w >= 740 && w <= 3840 && h >= 560 && h <= 2160)
                    o.screenshotSize = QSize(w, h);
            }
        }
    }
    return o;
}

QString ShellOptions::windowName() const {
    return files ? "Files" : settings ? "Settings" : control ? controlPage : "Preview";
}

QSize ShellOptions::windowSize() const {
    if (files)
        return {1200, 760};
    if (settings)
        return {960, 680};
    if (control)
        return {410, controlPage == "HarborMenu" ? 450 : 600};
    return {1440, 900};
}
