# Harbor 0.8.0 verification report

Date: 2026-09-24. Scope: the ten requested native System Settings areas and existing desktop regressions. Source includes current working-tree changes; this is not a claim of macOS equivalence or production hardware certification.

## Completed local checks

- Qt 6.11.2: 36 CTest suites passed, including private NetworkManager, BlueZ, AccountsService, CUPS and PackageKit fixtures; notification policies; storage hardlinks/symlinks; display guard, audio streams, battery helpers; gestures and accessibility.
- Python helper/session suite: 52 tests passed. These include interpreter isolation for the privileged charge helper, startup overrides, Flatpak argument validation, screen-reader preference restoration, and graceful session cleanup.
- Offline libnm import: 2 tests passed; synthetic WireGuard configuration is restricted to its user before any NetworkManager save.
- Runtime UI: 19 English pages, a desktop preview and five Arabic/dark pages at 740×560 passed QML diagnostics, dimensions and nonblank-content checks. Screenshots were visually inspected.
- Real KWin 6.7.4 on a private bus and virtual output: gesture handler Ready handshake, replacement mapping and unloading passed. No physical swipe was simulated.

## Debian package validation

**Result: passed, container exit 0.** Debian 13 completed all 36 CTest suites, 52 Python helper tests, 2 offline import tests and 3 runtime UI test cases. Real KWin 6.3 passed gesture initialization/reload/unload, window actions, notification delivery, Files menu export, two-output routing, keyboard switching/reload and installed-session graceful logout. Package installation and removal completed.

The release harness builds on Debian 13 / Qt 6.8 / KWin 6.3, runs the suites, installs the package, renders its UI, exercises private virtual KWin sessions and removes the package. Its final result is distributed with the source snapshot in `evidence/native-settings-0.8.0-trixie.txt`; successful completion is identified by `PACKAGE_INSTALL_RUNTIME_REMOVE_PASS`. No host installation is performed by this harness.

## Defects found and corrected during verification

- Blank Privacy/Startup pages caused by shadowing the QML child-data property.
- Lost credentials after a rejected incomplete authentication form.
- Optimistic switches retaining a failed mutation rather than service state.
- KWin QML reload failure caused by a cached generation directory.
- False inversion success when a compositor shortcut failed or did nothing.
- Screen-reader accessibility settings not restored on disable/exit.
- Session shutdown killing helper cleanup too early.
- Display status remaining busy after a dead helper; rollback claiming success without readback.
- File scans counting hardlinks twice or following directory aliases; logical rather than allocated file sizes.

## Limits and performance interpretation

Tests intentionally do not pair host Bluetooth devices, change network routes, create host accounts, print, alter firmware charging thresholds, suspend the host or install real system updates. Target-device checks remain necessary for those operations and authentication prompts. Virtual rendering does not measure GPU frame rate, physical input latency or battery drain. No performance percentage or production-readiness claim is made.

Long file enumeration runs off the UI thread, is cancellable and bounded to one million entries. Service operations have timeouts, and pages generally activate their polling only while visible. These design safeguards are verified where covered by tests, but do not replace measurements on the user's hardware.

See [Native Settings](NATIVE-SETTINGS.md) for exact service dependencies and unsupported platform capabilities, including new enterprise Wi-Fi profile creation and unrestricted Debian application permissions.

The container emitted non-fatal Qt thread-storage shutdown messages and an Xwayland socket warning in its minimal virtual session. The verified window/session paths were Wayland; this run is not certification of physical X11 application compatibility.
