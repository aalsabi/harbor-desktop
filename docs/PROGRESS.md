# SDD ledger — plan: docs/superpowers/plans/2026-09-23-harbor-kwin.md

- Execution authorized: inline, KWin + custom shell; no Plasma Shell.
- Ruling: projectless folder has no existing source repository; initialize a dedicated harbor-kwin repository inside the deliverable, no separate worktree needed. No existing user code is affected.
- Pre-flight 1→2→3: KWin protocol availability gates window model and dock; validate locally before declaring integration.
- Pre-flight 5→6: shared asynchronous operation model and persistent preferences, UI must read actual state after writes.
- Pre-flight 1→7: session owns only its children and its config; packaging must not change other desktop sessions.
- Ruling: dependencies are extracted locally for testing because global installation would alter the host. Packages built here remain sid builds unless independently built on trixie.
- Task 1: in progress. Local KWin executable absent; preparing isolated dependencies.
- Task 1: Ruling: KWIN_CONFIG is not a supported knob in the inspected KWin source; use a dedicated compositor XDG_CONFIG_HOME and restore app config in session-client. Regression test failed then passed, original kwinrc preserved.
- Task 2: offscreen UI render passes. Initial QML syntax and palette issues reproduced and corrected.
- Ruling: use GIO desktop-entry launching instead of KIO, preserving standard field-code handling and avoiding shell interpretation; fewer dependencies.
- Ruling: advanced network/audio/Bluetooth configuration initially opens explicitly named external tools; those flows remain integration gaps until replaced by native pages.
- Tasks 1–8: implementation delivered as experimental0.1.0 with native shell panels, KWin window model, original assets, settings/service adapters and packaging. Scope gaps are explicit in COMPATIBILITY.md; this is not completion of a production desktop.
- A Debian13 container was added after the extracted sid dependency check. The delivery package is built with Qt6.8/KWin6.3 dependencies, not copied from the sid build.
- Independent read-only review found three issues: cross-process preference updates, wrong default display mode, process-group cleanup after compositor exit. All three corrected; preference and supervisor regression tests added. Display mode binding selects currentModeId.
- Test adaptation: asynchronous window activation must be polled, not assumed after400ms; integration test now polls a bounded interval.
- Residual validation: actual hardware services, lock screen, portal screen sharing, hotplug, multi-monitor and accessibility remain unvalidated. No performance target is claimed from offscreen results.
- Clean Debian13 integration exposed missing KDE application menu index and relative Exec path. Added a Harbor-specific XDG menu and an absolute configured shell Exec. Verified KWin grants only declared window-management protocol with permission checks enabled.
- Final Debian13 validation: four Qt suites + five Python cases + UI render + virtual KWin operations + installed headless session startup + apt install/remove passed. X11, physical hardware and lock remain explicitly unvalidated.

- 0.1.1: corrected panel button overflow (38px buttons in a32px panel); dedicated28px panel buttons,38px panel, explicit Applications label, stronger contrast, bounded scrolling global-menu region, and reduced active-title width. UI render inspected; Debian13 validation in evidence/panel-0.1.1.txt.
- 0.2.0: added Harbor Files with three views, tabs/history, current-folder name filtering, mounted locations, bounded image/text preview, local file operations and dock/desktop registration. File operations use Qt + a separate Python worker and GIO trash; no KIO job dependency was added. Folder enumeration is still synchronous.
- Independent file-manager review identified Ctrl-selection toggling twice, file shortcuts capturing text edits, and missing broken symlinks. Corrected all three and added interaction/model regressions. The text-edit test also exposed location-field rebinding on unrelated state changes; fixed by updating it only on navigation.
- Files-specific limits: internal copy/cut clipboard, no Undo/merge/restore-trash UI/indexed search/remote protocols, partial-copy cleanup and cross-filesystem concurrent changes not transactionally guaranteed; documented in docs/FILES.md.
- Final0.2.0 verification passed on Debian13: six CTest suites (including GUI interaction), ten Python tests (including special-file copy refusal), package install, virtual KWin management of Files/settings windows, installed-session startup and removal. Full log: evidence/files-0.2.0.txt.
- 0.3.0 visual pass: inspected Apple macOS27 Finder reference, replaced stacked Files toolbars with one toolbar and compact sidebar, added accessible client-side window controls, neutral light/dark colors and blue accent, original blue folders/document icons, original gradient wallpaper, content-width dock. Full visual equivalence is not claimed; docs/VISUAL-MATCH.md records gaps.
- Review corrections: New tab remains mouse-accessible in the actions menu when tabs are hidden; sidebar/control buttons preserve keyboard/accessibility roles; selected toolbar glyph contrast corrected. Interaction tests exercise window action callbacks in addition to previous file workflows.
- Virtual KWin activation initially exceeded the3-second test poll. An8-second bounded readiness poll passed with the same activation assertion; this is a test timing adjustment, not a hardware performance guarantee. Preserved final integration log with the release.
- Final0.3 validation passed: six CTest suites, ten Python cases, preview render, package install/remove, Files/Settings enumeration/activation/close and installed-session startup. Inspected light Files, dark Files, Settings and composed desktop previews. Evidence: evidence/visual-0.3.0.txt.

## 0.3.1 — multiple monitor routing

A two-output KWin Wayland reproduction confirmed that both copies of each desktop, menu bar and dock were sent to the first output. QWindow::setScreen alone did not preserve the intended output through native window creation with the initial geometry at the origin. Set each window's initial position to the target screen's geometry origin before mapping it.

The regression test checks actual Wayland get_layer_surface output IDs, not only the stored intended QScreen. Before the fix: desktop/menu/dock each routed to [20,20]. After the fix: each routes to [20,22]. Tested with Debian 13 Qt 6.8 / KWin virtual two-output session; physical HDMI and hot-plug behavior still require device validation.

## 0.3.2 — functional Files menus in the panel

Implemented a DBusMenu exporter for Harbor Files with File/Edit/View/Go/Window. Each Files process registers its endpoint on its unique session-bus connection and associates it with its own Wayland surface through KWayland AppMenuManager. The shell reads the active window's exported address. File actions use the existing Files model and dialogs; unavailable selection/busy/modal actions are disabled and checked again at dispatch. Text editing commands target the last focused text control while the panel owns focus. Menu view states are rendered with check/radio markers.

Added a menu export/activation test, real-QML New Tab/view/text-paste/select-all interaction coverage, and a KWin integration assertion that the Files window advertises the endpoint and exports all five menu groups. The existing two-output routing regression remains in release validation. A composed preview was visually checked for panel labels; it is not a physical-display screenshot. Third-party applications still require their own DBusMenu export support.

## 0.4.0 — System Settings redesign and keyboard configuration

Replaced the sparse Settings page with a searchable category sidebar, grouped controls, original icon tiles, rounded selectors, real version/system information and Arabic labels/layout. Preserved existing hardware integrations and explicitly identified external-manager operations. Added an isolated XKB input-source helper and asynchronous Settings backend, with KWin reload/switch integration. See SETTINGS.md for the supported scope and gaps.

Validation includes eight CTest suites, fourteen Python cases, menu/window integration, two-output routing, a real virtual-KWin keyboard load/switch/reload test and package lifecycle checks. Hardware key-event/shortcut behavior is not inferred from the D-Bus test. No performance benchmark was added for this release.
