#!/bin/sh
set -eu
# Test harness only: intentionally operates inside a disposable Docker container.
test -f /.dockerenv || { echo "Run only inside the documented Docker test container" >&2; exit 2; }
apt-get install -y --no-install-recommends libkf6windowsystem-dev file libcap2-bin breeze-cursor-theme >/build/extra-deps.log 2>&1
cmake -S /src -B /build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build /build -j4
ctest --test-dir /build --output-on-failure
python3 -m unittest discover -s /src/tests -p 'test_*.py'
cd /build
cpack -G DEB
apt-get install -y --no-install-recommends ./harbor-desktop-0.5.2-Linux.deb >/build/install.log 2>&1
setcap -r /usr/bin/kwin_wayland || true
mkdir -p /tmp/runtime-harbor
chmod 700 /tmp/runtime-harbor
export LANG=C.UTF-8 XDG_MENU_PREFIX=harbor- XDG_RUNTIME_DIR=/tmp/runtime-harbor QT_QUICK_CONTROLS_STYLE=Basic KWIN_COMPOSE=Q HARBOR_BINARY=/usr/bin/harbor-shell
unset DISPLAY WAYLAND_DISPLAY
QT_QPA_PLATFORM=offscreen harbor-shell --preview --screenshot /build/trixie-preview.png
python3 /src/tests/ui_smoke.py
 timeout 40 dbus-run-session -- kwin_wayland --virtual --no-lockscreen --no-global-shortcuts --no-kactivities --exit-with-session /src/tests/kwin_integration.py
timeout 40 dbus-run-session -- kwin_wayland --virtual --output-count 2 --no-lockscreen --no-global-shortcuts --no-kactivities --exit-with-session /src/tests/multiscreen_integration.py
timeout 40 dbus-run-session -- python3 /src/tests/keyboard_integration.py
timeout 40 dbus-run-session -- python3 /src/tests/session_integration.py
apt-get remove -y harbor-desktop >/build/remove.log 2>&1
test ! -e /usr/bin/harbor-shell
echo 'PACKAGE_INSTALL_RUNTIME_REMOVE_PASS'
chown -R 1000:1000 /build
