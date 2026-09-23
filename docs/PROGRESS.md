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
