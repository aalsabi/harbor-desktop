# Licensing and provenance

Harbor source code, QML, original SVG artwork, theme configuration, scripts and documentation: GPL-3.0-or-later. Copyright 2026 Harbor Desktop contributors. The complete GPL version 3 text is included in LICENSE.

The SVG backgrounds and six Harbor icons were authored for this project from simple geometry. No Apple image, font, sound, logo or extracted system resource is included. Golden Gate is only the reference platform named in the design documentation. This project is independent of Apple and KDE.

External libraries and services are installed by Debian, not vendored:
- Qt 6 (Core, Gui, Quick, Controls, DBus, SVG): their available free-software licenses; this build uses shared Debian libraries.
- KWin, KWayland, LayerShellQt, KWindowSystem, KScreen and Breeze: KDE free-software components under their respective GPL/LGPL licenses. Package copyright files are authoritative.
- GLib/GIO: LGPL-2.1-or-later.
- NetworkManager, BlueZ, PipeWire/WirePlumber, AccountsService, Polkit, portals, Python and Linux: their respective free-software licenses in Debian.
- Noto fonts, when installed: SIL Open Font License.
- Icons provided by installed applications retain their own licenses; Harbor reads them at runtime and does not redistribute them in this source archive.

The names of KDE APIs and packages containing “Plasma” do not imply execution of Plasma Shell. Harbor does not run plasmashell or startplasma-wayland. The Breeze window decoration is reused and configured, not represented as original Harbor artwork. The Harbor color scheme is original.
