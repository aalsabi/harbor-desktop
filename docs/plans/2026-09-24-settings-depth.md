# Settings depth implementation plan

Goal: complete the five explicitly requested gaps in Harbor's native UI, using open-source Debian services and measured, honestly labelled results.

Architecture: extend NetworkSettings for EAP and Printers for discovery; separate PrinterDrivers handles repository package transactions. ApplicationStorage performs worker scans and explicit attribution. NotificationPreferences owns Focus profiles and scheduling so shell and settings share policy. An unprivileged Bubblewrap launcher provides opt-in isolated native application launches; it never changes permissions on an already running ordinary process.

Constraints: Debian13/Qt6.8/KWin, English/Arabic, preserve all existing work and independent UI-language/regional formats, no tooltips, no host account/network/package/hardware mutations during tests. Package installations and sandbox launches require explicit user action in the product. No downloaded vendor binaries. Do not claim universal sandbox compatibility or perfect storage attribution.

- [x] Enterprise networking: test validated EAP-TLS/PEAP/TTLS settings and certificate/domain requirements before implementing native edit/create forms. Verify exact NM setting payloads and secret preservation with private D-Bus fixtures.
- [x] Printers: test CUPS discovery parsing/selection and driver matching. Resolve supported Debian driver packages, simulate trusted installation, require preview confirmation, expose progress/errors, and refresh available drivers after success.
- [x] Application storage: test allocated bytes, links, shared files, cancellation and uncertain attribution. Measure owned binaries and explicitly associated user-data/cache folders; show excluded/shared/unknown separately. No cleanup action.
- [x] Focus: test multiple profiles, weekly overnight schedules and manual overrides; preserve notification history while filtering attention by profile allowlist. Native create/edit/delete and day/time controls, persisted across processes.
- [x] Native privacy: test strict desktop Exec expansion, isolation arguments, no host D-Bus/X11 sockets, minimal environment, home/data separation, optional network/audio/camera/folder grants and failure without unsandboxed fallback. Native profile editing and explicit isolated launch; ordinary launches remain visibly outside this policy.
- [x] Integrate contexts/resources/dependencies, review privilege boundaries, run complete local suites and actual UI rendering.
- [x] Build/install/run/remove0.9.0 in disposable Debian container, export package/source/checksums and test report including unsupported capabilities.

Review focus: authentication-server identity, stale printer discovery/driver package identifiers, symlink or shared-directory attribution, overnight day ownership and profile changes from another process, and sandbox escape through exported sockets/environment. Each owner covers these with isolated regression fixtures.
