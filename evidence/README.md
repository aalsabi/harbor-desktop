# Evidence index

- indicator-0.5.2.txt: Debian13 full release checks with thirteen Qt suites and package lifecycle.
- indicator-local-0.5.2.txt: local regressions including compositor-driven language changes, click-cycle backend and service-loss handling.

- keyboard-fix-0.5.1.txt: Debian13/KWin6.3 full release checks, twelve Qt suites and package lifecycle.
- keyboard-local-0.5.1.txt: twelve local regression suites, including production reload notifications.
- keyboard-kwin67-0.5.1.txt: isolated local KWin6.7 keyboard load/switch/reload; virtual compositor warnings are preserved.

- control-0.5.0.txt: Debian13 final build, eleven Qt suites, fourteen Python cases, virtual KWin/menu/multiscreen/keyboard/session logout and package lifecycle.
- control-local-0.5.0.txt: eleven local Qt suites including the slider debounce regression.

- settings-0.4.0.txt: final Debian13 Settings release checks, including eight CTest suites, fourteen Python cases and real virtual-KWin keyboard load/switch/reload.
- settings-local-0.4.0.txt: local Qt6.11 UI/regression checks.

- menus-0.3.2.txt: seven CTest suites, ten Python tests, Files menu export through KWin, two-output routing, installed-session and package install/remove checks.

- multiscreen-0.3.1.txt: two-output Wayland routing regression, full unit/UI checks and package install/runtime/remove verification.

- visual-0.3.0.txt: release0.3 final build, UI interaction, packaging, KWin integration, install and remove checks.
- files-0.2.0.txt: release0.2 verification.
- trixie-validation.txt: baseline0.1 Debian13 build, tests, package installation, UI render, virtual KWin integration, installed-session startup and removal run. Read its final status; intermediate attempts are not acceptance evidence.
- benchmark.json: one local Qt6.11 preview-only offscreen idle measurement; no KWin/GPU/hardware performance claim.
- trixie-build.txt: earlier successful Debian13 build/package.
- kwin-probe.txt: earlier local KWin6.7 protocol probe.
- *-red.txt and kwin-integration-initial-failure.txt: intentionally preserved earlier failures, not final results.
- Other *-green.txt / unit-tests.txt / python-tests.txt: earlier development checks. Final verification supersedes them.

The virtual compositor uses software rendering. Test-only --no-lockscreen disables locking for integration tests, and a file capability is removed from the container copy of KWin so it can run under Docker's capability restrictions. Neither action modifies the host or the delivered KWin package. These runs do not validate a secure lock screen.
