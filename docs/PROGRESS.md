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
