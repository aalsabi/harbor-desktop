# System Settings — Harbor 0.4.0

Open the Settings icon in the Dock. The redesigned window uses a searchable sidebar, original coloured category tiles, grouped controls, rounded fields, light/dark appearance and Arabic layout. It is inspired by macOS System Settings, not an Apple component or a claim of exact parity.

## Available pages

- General: interface language, About and Software Update links.
- Wi-Fi / Network: radio control, active connections, nearby networks and connection-editor link.
- Bluetooth: power and device list; pairing uses the external manager.
- Sound: default output volume and mute; device routing uses the external mixer.
- Appearance: Harbor light/dark appearance; other apps keep their themes.
- Accessibility: reduce Harbor animation.
- Desktop & Dock: original wallpaper preview, panel opacity and pinning guidance.
- Displays: brightness, per-output mode/scale and the existing 15-second rollback guard.
- Keyboard: input-source selection/order, switching shortcut and a typing test field.
- Battery: supported power profiles (not battery-health or charging-limit controls).
- Users & Groups: AccountsService names and account type; authenticated name editing.
- About: actual Harbor version, system name and CPU architecture.

Disabled hardware controls indicate an unavailable service. Network pairing, advanced audio routing and software updates still open named external Debian tools. Account creation/password changes, additional accessibility tools and Apple-specific services are not implemented. There are no Apple account or iCloud placeholders.

## Keyboard

Choose one to four distinct base layouts from the installed XKB catalogue. English (US) is `us`; Arabic is `ara`. Move a source upward to change its default order. Choose Alt+Shift, Ctrl+Shift, Super+Space, Ctrl+Space or no XKB switching shortcut, then Apply. “Switch now” uses KWin's keyboard service. The typing field lets you check the result.

Only `~/.config/harbor/session/kxkbrc` is written, through an atomic replacement. Existing variants of retained layouts and unrelated keyboard options are preserved. Configurations containing multiple variants of the same base layout cannot be edited here yet: the helper refuses them without changing the file. This UI does not configure compose-key options, repeat rate or input-method engines such as Fcitx/IBus.

Inside Harbor, Apply requests KWin's `org.kde.keyboard` reloadConfig signal. Outside Harbor, it saves for the next Harbor session. The message distinguishes a saved/requested change from a verified physical keyboard result. The session-specific configuration does not change GDM or other desktops' input settings. Custom shortcuts may conflict with application shortcuts.

Direct page launch: `harbor-settings Keyboard` or `harbor-settings Appearance`.

## Validation and references

Automated UI checks cover sidebar navigation/search, input-source add/apply, light/dark appearance and Arabic layout rendering. Helper tests cover validation, variant preservation and refusal of ambiguous existing variants. A separate isolated KWin integration test loads US/Arabic layouts, changes the active source and reloads US/German settings. Physical key-event shortcuts and particular hardware remain unverified.

- Apple System Settings layout: https://support.apple.com/en-euro/guide/mac-help/mh15217/mac
- Apple input-source settings: https://support.apple.com/en-za/guide/mac-help/-mchl84525d76/mac
- KWin keyboard D-Bus/reload contract: https://github.com/KDE/kwin/blob/Plasma/6.3/src/keyboard_layout.cpp
- KWin XKB configuration: https://github.com/KDE/kwin/blob/Plasma/6.3/src/xkb.cpp

No Apple artwork, fonts, screenshots or implementation code were copied into the product.
