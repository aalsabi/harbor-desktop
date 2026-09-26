# Complete native System Settings implementation plan

**Goal:** Implement the user's ten explicitly requested remaining areas inside Harbor, preserving the English-interface/Saudi-region split and KWin session isolation.

**Architecture:** Separate lazy Qt service objects and native QML pages. D-Bus services own privileged actions with system authentication; fixed-argument helpers are used only where no suitable service exists. Persistent user settings have validated, atomic writes. Independent work is split by backend ownership; shared navigation, resources, session startup and packaging are integrated centrally.

**Platform:** Debian 13+, Qt 6.8+, KWin Wayland, no Plasma Shell, GPL-compatible dependencies and original UI. No hover tooltips. No real host account/network/hardware/update mutations in development tests.

## Independently testable implementation units

- [x] NetworkSettings/NetworkPage: NetworkManager saved and new Wi-Fi/wired profiles, secrets only in memory or protected stdin, validated IPv4/IPv6/DNS, supported VPN import/connect. Test malformed addresses, service denial, stale device IDs and secret non-disclosure with injected services.
- [x] Bluetooth/BluetoothPage: BlueZ Agent1 registration, asynchronous PIN/passkey confirmation/cancel, explicit pairing and removal confirmation. Test foreign prompt sender/device, cancel, duplicate request and delayed replies on private bus.
- [x] Printers/PrintersPage: cups-pk-helper authorized creation/removal/driver changes, CUPS options and selected-job cancel. Validate URI, printer and queue identities. Fake-command/private-bus tests only.
- [x] Accounts/AccountsPage: authorized create/delete/type/password methods; confirmation for deletion; home deletion off by default; protect own/root/last admin. Passwords hashed in memory, no command arguments or logs. Private AccountsService fixture tests.
- [x] Notifications settings: persisted per-app delivery/preview options and Do Not Disturb, shared across Settings and shell processes. Test suppression, restoration, replacements and retention with temporary config.
- [x] DisplaySettings/DisplaysPage: positions/primary/mode/scale through KScreen with independent rollback watchdog; supported DDC brightness. Test invalid topology/IDs and guard timeout using fake tools.
- [x] AudioStreams/AudioStreamsPage: actual stream volumes/mutes and sink routing through PipeWire's PulseAudio interface. Test ID validation and command errors.
- [x] PowerSettings/PowerPage: idle display/suspend policy backed by a session-owned agent; supported sysfs charging thresholds via a narrowly scoped authenticated helper. Test threshold ordering, path validation and session child cleanup.
- [x] Advanced settings: real XDG startup application toggles/selection and session execution; Flatpak permission overrides with accurate limits for unsandboxed apps; PackageKit update transactions inside Harbor; supported KWin gesture/effect controls and accessibility integration. Test file containment, unsupported services, canceled authorization and available capabilities.
- [x] Storage analysis: cancellable worker enumeration under selected user folders, no symlink traversal, categories/top files and installed application size where available. Test symlink escape, unreadable paths, hardlinks and cancellation. No destructive cleanup.

## Integration and verification

- [x] Wire new page routes and context objects in Settings.qml/main.cpp/resources.qrc, preserving the current working tree.
- [x] Register dedicated tests in CMakeLists.txt and update packaging dependencies/helpers/session ownership.
- [x] Build: `cmake --build work/build -j4` from the workspace root.
- [x] Run isolated suites: `ctest --test-dir work/build --output-on-failure` and Python discovery under tests.
- [x] Render native pages in English/light and Arabic/dark, check narrow windows and error states.
- [x] Build/install/run/remove inside the existing disposable Debian 13 container; never reboot/logout or change host settings during tests.
- [x] Deliver package, source snapshot, test evidence and exact remaining hardware/platform limitations. Do not label unavailable capabilities as implemented or assert macOS parity.
