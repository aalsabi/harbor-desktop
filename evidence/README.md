# Evidence index

- locale-generation-0.6.1.txt: nineteen Qt suites, thirty-seven Python cases, actual installed root-helper generation of ar_SA/en_US in the isolated Debian13 container, persistence/idempotency, invalid/unprivileged rejection, unchanged system default locale, existing KWin integrations and package install/remove (exit0).
- locale-generation-local-0.6.1.txt / locale-generation-python-0.6.1.txt / locale-generation-ui-0.6.1.txt: local verification, including confirmation, cancel/retry/busy/save UI flow and Arabic small-window confirmation bounds. Authentication completion is mocked in UI tests; the physical password dialog is not automated.
- locale-generation-helper-red.txt: intentionally failing tests before fixed cwd and concurrent configuration edit protection.

- region-0.6.0.txt: final Debian13/Qt6.8 build, nineteen Qt suites, twenty-eight Python cases, UI rendering, KWin menu/multiscreen/keyboard/session integrations and package lifecycle; container exit0.
- region-local-0.6.0.txt / region-python-0.6.0.txt: final local suites. UI tests cover drafts, cancellation, ordering, 12/24-hour changes, persistence and Arabic dialog footer bounds.
- region-measurement-red.txt: intentional pre-fix reproduction of a missing visible measurement-locale fallback warning.

- no-tooltips-0.5.5.txt: Debian13 build, seventeen Qt suites, seventeen Python cases, UI rendering, virtual KWin integrations and package lifecycle passed (exit0). All Harbor QML tooltip declarations removed; existing pointer-interaction tests pass.

- launch-0.5.4.txt: Debian13 release build; seventeen Qt suites, seventeen Python cases, virtual KWin integrations, installed session/logout and package install/remove passed (exit0).
- launch-local-0.5.4.txt / launch-python-0.5.4.txt: local verification of launch coalescing, focus, failure retry, child pipe lifetime, terminal selection, actual Dock clicks and selected activation-environment import.
- launch-duplicate-red.txt / launch-review-red.txt: intentionally failing pre-fix reproductions of duplicate processes, hidden terminal preferences and inherited pipe lifetime.

- input-and-icons-0.5.3.txt: Debian13 release checks with fifteen Qt suites, fourteen Python cases, six popup types, multiscreen/keyboard/session integrations and package lifecycle. Container completed with exit0.
- input-and-icons-local-0.5.3.txt: final local suite including real pointer menu interaction, source editor Cancel/Done, icon precedence/path/configuration and standard-directory discovery.

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
