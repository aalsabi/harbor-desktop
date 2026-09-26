# Harbor 0.7.0 validation — 2026-09-24

## Results

- Local Qt 6.11.2: **27/27 CTest suites passed**, including the six new backend/UI suites. Total suite runtime: 19.12 seconds.
- Debian 13 / Qt 6.8.2: **27/27 CTest suites passed**. Total suite runtime: 10.94 seconds.
- Python regressions: **38/38 passed** locally and on Debian 13, including English-only interface with Saudi regional categories.
- Installed and local native-page smoke checks: preview plus all six direct Settings pages rendered successfully without QML reference/type errors.
- Debian package `harbor-desktop 0.7.0 amd64`: built, installed, exercised, and removed successfully in the disposable container.
- Virtual KWin tests passed for menus/windows, two-output routing, keyboard layout load/switch/reload, managed Settings window and graceful test-session logout.
- Installed locale helper tests passed in the container: valid locale generation/persistence, repeated invocation, invalid/unprivileged rejection and unchanged system default locale.

Logs: [local](../evidence/native-settings-0.7.0-local.txt), [Debian](../evidence/native-settings-0.7.0-trixie.txt), [Python](../evidence/native-settings-0.7.0-python.txt), [runtime UI](../evidence/native-settings-0.7.0-runtime-ui.txt).

## UI review

All six new pages were rendered with representative fixture data in English/light at 960×720 and Arabic/dark at 740×560. Search and navigation, including reusing the Mouse/Touchpad component, were verified. Initial loading errors were corrected before the final all-green run. Screenshots use test devices and are not claims about attached physical hardware.

## Performance and safety boundaries

The suite times above measure tests, not application startup latency. Storage enumeration runs on a worker; pointer polling and the live clock timer operate while their settings page is open. No new recurring full-disk scan was added. Physical hardware performance and input-to-display latency were not benchmarked.

Mutating service tests use private D-Bus fixtures, temporary XDG defaults and fake CUPS commands. Real package installation and locale generation happened only inside the disposable Debian container. The host desktop was not restarted, its system clock/input/printer/default-application settings were not changed, and the release package was not installed on the host.

No claim is made for physical printer setup, real printing, secure lock, X11/XWayland applications, or real administrator password dialogs. The virtual environment emitted expected missing-cursor/X11-socket warnings; the tests listed above exercise Wayland behavior. Feature limits are documented in [Native Settings](NATIVE-SETTINGS.md).

## Source snapshot

The source archive is a snapshot of the working tree based on commit `9130bdf`. It retains the pre-existing unreleased Bluetooth/menu changes and includes this native Settings expansion. No existing uncommitted work was discarded or committed on the user's behalf.
