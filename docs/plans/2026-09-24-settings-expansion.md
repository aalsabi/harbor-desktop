# Native settings expansion

User requirement: add missing System Settings functionality inside Harbor with the same grouped, searchable, English/Arabic, light/dark design. Do not delegate these new pages to KDE settings windows. Keep KWin, existing desktop isolation, locale separation, and the no-tooltip policy.

## Deliverable

- Date & Time: confirmed timedate1 state, timezone selection, automatic synchronization, validated manual time with interactive system authorization.
- Mouse and Touchpad: enumerate real KWin input devices, show supported controls only, apply through KWin's persistent device properties.
- Default Applications: choose installed handlers using GIO; show failures and actual saved handlers.
- Storage: mounted filesystem capacity/available space with refresh and opening supported local locations; no destructive cleaning.
- Printers: installed queues/status, user default printer, and queue information using CUPS. No simulated printer discovery or unsupported setup controls.

Each page is loaded on demand and backed by a separate native service object. Navigation and direct page launch share the same names. Missing hardware or service is an explicit unavailable state. No actual clock, input device, printer, locale, account or default application is changed while testing.

## Validation

1. Isolated backend tests for validation, service errors and changes against private D-Bus/mock commands or temporary XDG configuration.
2. Native-page UI loading, search, navigation, English/Arabic and light/dark, including a 740×560 window.
3. Existing complete Qt/Python regression suites.
4. Debian 13 build/package and virtual KWin integration if available.
5. Document remaining feature gaps and untested physical hardware. Do not call this full macOS parity.

## Existing work preserved

The initial worktree already contains an unreleased 0.6.3 Bluetooth D-Bus/menu and recent-items change. This expansion preserves it. Language/Region behavior from 0.6.2 remains unchanged.
