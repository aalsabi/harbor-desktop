# Evidence index

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
