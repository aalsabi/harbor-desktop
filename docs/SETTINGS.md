# System Settings — Harbor 0.5.0

Open the Settings icon in the Dock. The redesigned window uses a searchable sidebar, original coloured category tiles, grouped controls, rounded fields, light/dark appearance and Arabic layout. It is inspired by macOS System Settings, not an Apple component or a claim of exact parity.

## Available pages

- General: Language & Region, About/Software Update links and confirmed logout/restart/shutdown.
- Wi-Fi / Network: radio control, saved-connection connect/disconnect, nearby SSIDs with signal/security and connection-editor link.
- Bluetooth: power, paired-device names and connect/disconnect; new pairing uses the external manager.
- Sound: default output/input selection, separate volume/mute controls; per-application routing uses the external mixer.
- Appearance: Harbor light/dark appearance and five accent colours; other apps keep their themes.
- Accessibility: reduce Harbor animation and transparency.
- Desktop & Dock: three original vector wallpapers, dock icon size, panel opacity and pinning guidance.
- Displays: brightness, per-output mode/scale and the existing 15-second rollback guard.
- Keyboard: input-source selection/order, switching shortcut and a typing test field.
- Battery: UPower battery percentage/charging state and supported power profiles (not battery-health or charging-limit controls).
- Users & Groups: AccountsService names and account type; authenticated name editing.
- About: actual Harbor version, system name and CPU architecture.

Disabled hardware controls indicate an unavailable service. Network pairing, advanced audio routing and software updates still open named external Debian tools. Account creation/password changes, additional accessibility tools and Apple-specific services are not implemented. There are no Apple account or iCloud placeholders.

## Keyboard

Open Keyboard → Text Input → Edit. The input-source sheet has a configured-source list, selected-source details, a searchable + chooser and removal/default controls. Cancel discards edits; Done saves them. Choose one to four distinct base layouts from the installed XKB catalogue. English (US) is `us`; Arabic is `ara`. Move a source upward to change its default order. Choose Alt+Shift, Ctrl+Shift, Super+Space, Ctrl+Space or no XKB switching shortcut, then Done. The panel input menu uses KWin's keyboard service. The typing field lets you check the result.

Only `~/.config/harbor/session/kxkbrc` is written, through an atomic replacement. Existing variants of retained layouts and unrelated keyboard options are preserved. Configurations containing multiple variants of the same base layout cannot be edited here yet: the helper refuses them without changing the file. This UI does not configure compose-key options, repeat rate or input-method engines such as Fcitx/IBus.

Inside Harbor, Apply requests both the older `org.kde.keyboard.reloadConfig` signal and the newer `org.kde.kconfig.notify.ConfigChanged` notification for `/kxkbrc`. KWin 6.7 uses KConfigWatcher and no longer listens to the old reload signal. Outside Harbor, it saves for the next Harbor session. The message distinguishes a saved/requested change from a verified physical keyboard result. The session-specific configuration does not change GDM or other desktops' input settings. Custom shortcuts may conflict with application shortcuts.

Direct page launch: `harbor-settings Keyboard` or `harbor-settings Appearance`.

## Validation and references

Automated UI checks cover sidebar navigation/search, input-source add/apply, light/dark appearance and Arabic layout rendering. Helper tests cover validation, variant preservation and refusal of ambiguous existing variants. A separate isolated KWin integration test loads US/Arabic layouts, changes the active source and reloads US/German settings. Physical key-event shortcuts and particular hardware remain unverified.

- Apple System Settings layout: https://support.apple.com/en-euro/guide/mac-help/mh15217/mac
- Apple input-source settings: https://support.apple.com/en-za/guide/mac-help/-mchl84525d76/mac
- KWin keyboard D-Bus/reload contract: https://github.com/KDE/kwin/blob/Plasma/6.3/src/keyboard_layout.cpp
- KWin XKB configuration: https://github.com/KDE/kwin/blob/Plasma/6.3/src/xkb.cpp

No Apple artwork, fonts, screenshots or implementation code were copied into the product.

## Control Center and shared state (0.5.0)

Battery reporting requires the Debian `upower` service; if it is absent, battery readings remain unavailable.

Control Center provides radio toggles, actual backlight/output-volume values, mute, keyboard switching, supported power profiles, battery status and direct links to the corresponding Settings page. Lock is enabled only when a ScreenSaver service is registered; this is availability detection, not secure-lock certification. Session actions show a confirmation dialog. Logout exits the Harbor shell through its session D-Bus endpoint, allowing the existing session supervisor to stop KWin. Restart/shutdown use login1 permission checks and interactive authentication where supported.

Settings and Control Center use the same SystemServices data model in the shell. Separate Settings processes read the same device services; refresh occurs every 15 seconds and after completed actions, so external changes are not instantaneous. Command writes are serialized, repeated pending slider writes coalesce, and debounce handlers capture the requested value independently of state refreshes. The thumb can briefly reflect an older snapshot while a service refresh completes. Errors remain visible. Brightness targets the backlight class, not keyboard LEDs; external-monitor DDC brightness is not implemented.

Network credentials and new Bluetooth pairing still require external managers. Users & Groups supports existing account information/name editing, not full account/password administration. Software Update launches Discover. Accessibility does not provide a complete screen reader/magnifier suite. Media transport, Focus, AirDrop, screen mirroring and Apple online services are not implemented. This release improves every existing category's supported scope; it does not implement every macOS setting.

Validation: 11 Qt suites, including normalized service output, queued slider writes, state-refresh/debounce regression and logout confirmation. Session integration verifies graceful logout in a private virtual KWin session. Real wireless, Bluetooth, audio routing, battery hardware, restart/shutdown authorization and physical monitor backlights still require testing on the target machine. Tests never reboot or power off the host.

Service contracts: [wpctl](https://pipewire.pages.freedesktop.org/wireplumber/man/wpctl.html), [pw-dump](https://docs.pipewire.org/page_man_pw-dump_1.html), [nmcli](https://networkmanager.pages.freedesktop.org/NetworkManager/NetworkManager/nmcli.html), [BlueZ](https://hadess.github.io/bluez/bluetoothctl.html), [UPower](https://upower.pages.freedesktop.org/upower.freedesktop.org/docs/UPower.html).

## Keyboard reload fix (0.5.1)

A real KWin 6.7 session retained US only despite a saved US/Arabic configuration. Sending the old reload signal did not change the live layout list. Sending the KConfig notification immediately loaded both sources, and activating Arabic returned success. The application now emits both protocols to support older and newer KWin. A private-bus regression exercises the production Keyboard.apply path and checks notification payloads.

References: [KWin 6.7 keyboard watcher](https://github.com/KDE/kwin/blob/Plasma/6.7/src/keyboard_layout.cpp), [KConfig notification contract](https://github.com/KDE/kconfig/blob/master/src/core/kconfigwatcher.cpp).

## Panel input indicator (0.5.2)

The top panel reads the active KWin input source, independently of the interface language or saved configuration. Arabic shows ع, US/GB English shows EN, and other layouts show their XKB code. Hover shows the full name and variant. Click opens a selectable input menu with a checkmark on the current source and a Keyboard Settings link. Layout/list-change signals refresh the indicator, with a five-second retry for service recovery. Missing service or an invalid active index clears the stale value to a disabled keyboard symbol.

## Input menu and system icons (0.5.3)

The language indicator no longer creates a hover tooltip or cycles blindly. It opens a separate panel menu that calls KWin.setLayout for the chosen source and closes only after success; errors remain visible. The Settings source manager follows the Text Input → Edit organisation documented by [Apple](https://support.apple.com/en-nz/guide/mac-help/mchl84525d76/mac), with original UI code. It does not implement macOS-specific text correction, dictation or a keyboard-layout preview.

Application icons use Harbor’s explicit `iconTheme` preference (when set), otherwise the configured KDE icon theme and desktop-file icon paths, including paths with spaces and #. Bundled Harbor artwork is fallback only. Running windows resolve icons by desktop ID or StartupWMClass. A missing or unidentifiable application may still show the generic fallback. The icon theme is read at application startup; live external theme-change propagation is not implemented.

An already-installed theme such as `MacTahoe` can be selected with `iconTheme=MacTahoe` in Harbor settings.ini. The package does not redistribute that third-party theme.

Icon discovery explicitly includes standard XDG data icon directories, ~/.icons and pixmaps fallbacks so it works without Plasma’s platform-theme plugin.

## Language & Region — 0.6.0

Dedicated page with ordered language preferences, locale selection and General/Dates/Times advanced sheets. See LANGUAGE-REGION.md for persistence, Debian session integration and exact scope limits.
