# Harbor 0.9.0 verification report

Date: 2026-09-24. Scope: enterprise 802.1X, printer discovery/repository drivers, opt-in native application isolation, scheduled Focus profiles and measured application storage. This report covers the working-tree source distributed with this release.

## Local verification

- 39 CTest suites cover desktop regressions, private service fixtures and native QML forms. Focus editing is exercised through the actual QML component.
- 61 Python helper tests cover session handling and settings helpers, including nine sandbox tests and an actual Bubblewrap containment probe.
- Runtime rendering covers 19 English settings pages, seven Arabic/dark pages at 740×560 and the desktop preview. No QML diagnostic or blank-content failures; representative screenshots visually inspected.
- A real Qt Wayland window was exposed inside Bubblewrap on a private virtual KWin. The probe verified its private home and absent host bus. No real display or user application was used.
- The application storage suite includes 11 passing cases (including Qt setup/cleanup): allocated blocks, sparse files, symlinks, hardlinks, shared ownership, same-app cross-category deduplication and cancellation/associations.

## Debian validation

**Passed: container exit 0, PACKAGE_INSTALL_RUNTIME_REMOVE_PASS.** All 39 CTest suites passed. Python ran 61 tests with one explicit namespace-restriction skip; the other 60 passed. Two offline VPN import tests and all three UI rendering cases passed. The separate sandbox GUI probe also explicitly skipped in Docker because user namespaces are prohibited; both sandbox probes passed locally.

Installed KWin 6.3 integration passed gesture initialization/reload/unload, window activation/close, notification delivery, Files menu export, two-output routing, keyboard switching/reload and graceful session logout. Locale generation was verified for English and Arabic without altering the default locale. Non-fatal thread-storage shutdown and minimal Xwayland socket warnings occurred; physical X11 compatibility is not certified.

The disposable Debian 13 harness builds with Qt 6.8 and KWin 6.3, runs all suites, installs the generated package, renders installed UI, exercises virtual compositor/session integration and removes the package. Final results are recorded in evidence/settings-depth-0.9.0-trixie.txt. The package is not installed on the user's host by this process.

## Regression findings corrected

- Launching an application with a newly enabled isolation profile could focus an already running unisolated window. Harbor now refuses unknown pre-existing windows and asks the user to close them; it never silently launches outside isolation.
- Sandbox runtime configuration exposed too much of /etc; only selected public runtime files are mounted. Symlink storage roots are rejected before directory creation and child output cannot grow unbounded logs.
- Focus updates from a second settings process could overwrite completed external edits; mutations synchronize persisted state first.
- Hardlinked files belonging to one app across data/cache categories could be incorrectly classified as shared; these remain attributed once to that app.

## Limits and performance

Real RADIUS authentication, physical printer discovery/output and installation of model-specific driver packages were not performed against the user's devices. NetworkManager, CUPS and PackageKit contracts are tested using isolated fixtures; target-device validation remains necessary. Only trusted repository driver families are offered, with a transaction preview and explicit installation confirmation. No proprietary vendor driver is downloaded.

Native isolation applies to compatible new Wayland applications launched from Harbor. It does not retrofit restrictions to ordinary running processes or external launchers. Granting network also permits host-local network services; granting audio can permit microphone capture. No D-Bus, X11, GPU acceleration or portals are supplied by the default profile, so application compatibility is limited. See SANDBOX-SECURITY.md.

Focus controls Harbor notification attention, retains history and does not silence arbitrary audio produced by applications. Schedules use local time; overlapping schedules use the first matching profile.

Storage measures allocated regular-file blocks, not dpkg estimates. Shared files are counted once; unknown native data/cache requires explicit folder attribution. Dependencies without desktop ownership and Flatpak runtimes are excluded. Directory metadata, compression and shared reflink extents prevent these figures from being exact reclaimable physical storage. Scanning runs off the UI thread, is cancellable and bounded to 500,000 entries; no files are deleted.

Virtual rendering and fixture timings do not establish physical GPU frame rate, battery drain, network throughput or printer compatibility. No percentage equivalence or production hardware certification is claimed.
