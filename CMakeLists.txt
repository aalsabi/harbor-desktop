cmake_minimum_required(VERSION 3.22)
project(HarborDesktop VERSION 0.9.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Quick QuickControls2 DBus Network Test Svg LinguistTools)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-unix-2.0)
find_package(LayerShellQt REQUIRED)
find_package(KWayland REQUIRED)
find_package(KF6WindowSystem REQUIRED)

# ---------------------------------------------------------------------------
# Libraries
# ---------------------------------------------------------------------------
# harbor-core: preferences, commands, region and translations; depends on QtCore only.
add_library(harbor-core
    src/core/Command.cpp
    src/core/Preferences.cpp
    src/core/Region.cpp
    src/core/Translation.cpp
)
target_include_directories(harbor-core PUBLIC src)
target_link_libraries(harbor-core PUBLIC Qt6::Core)

# harbor-desktop-core: system services, window management and native settings back ends.
add_library(harbor-desktop-core
    # Shell services
    src/core/Accounts.cpp
    src/core/Applications.cpp
    src/core/Bluetooth.cpp
    src/core/Files.cpp
    src/core/FilesMenu.cpp
    src/core/GlobalMenu.cpp
    src/core/Keyboard.cpp
    src/core/Notifications.cpp
    src/core/SystemServices.cpp
    src/core/SystemState.cpp
    src/core/Tray.cpp
    src/kwin/WindowModel.cpp
    # Native System Settings pages; services are queried only when opened.
    src/core/AccessibilitySettings.cpp
    src/core/ApplicationStorage.cpp
    src/core/AudioStreams.cpp
    src/core/DateTimeSettings.cpp
    src/core/DefaultApps.cpp
    src/core/DisplaySettings.cpp
    src/core/GestureSettings.cpp
    src/core/NetworkSettings.cpp
    src/core/NotificationPreferences.cpp
    src/core/PointerSettings.cpp
    src/core/PowerSettings.cpp
    src/core/PrinterDrivers.cpp
    src/core/Printers.cpp
    src/core/SoftwareUpdate.cpp
    src/core/StorageSettings.cpp
    src/core/UserSettings.cpp
)
target_include_directories(harbor-desktop-core PUBLIC src)
target_link_libraries(harbor-desktop-core PUBLIC
    harbor-core Qt6::Gui Qt6::DBus Qt6::Network Plasma::KWaylandClient PkgConfig::GIO)

# ---------------------------------------------------------------------------
# Translations
# ---------------------------------------------------------------------------
# `cmake --build <dir> --target update_translations` refreshes the .ts files from the sources;
# edit them with Qt Linguist. The compiled .qm files are embedded under :/i18n in harbor-core
# so the shell and the UI tests load the same catalogue.
set(HARBOR_TS_FILES translations/harbor_ar.ts)
file(GLOB HARBOR_TR_SOURCES CONFIGURE_DEPENDS qml/*.qml src/*/*.cpp src/*/*.h)
qt_add_lupdate(TS_FILES ${HARBOR_TS_FILES} SOURCES ${HARBOR_TR_SOURCES}
    LUPDATE_TARGET update_translations NO_GLOBAL_TARGET OPTIONS -locations none -no-obsolete)
qt_add_lrelease(TS_FILES ${HARBOR_TS_FILES} QM_FILES_OUTPUT_VARIABLE HARBOR_QM_FILES NO_GLOBAL_TARGET)
qt_add_resources(harbor-core harbor_translations PREFIX /i18n BASE ${CMAKE_CURRENT_BINARY_DIR}
    FILES ${HARBOR_QM_FILES})

# ---------------------------------------------------------------------------
# Shell
# ---------------------------------------------------------------------------
add_executable(harbor-shell
    src/shell/main.cpp
    src/shell/Controller.h
    src/core/Icons.cpp
    src/kwin/WindowMenu.cpp
    resources.qrc
)
target_compile_definitions(harbor-shell PRIVATE HARBOR_VERSION="${PROJECT_VERSION}")
target_link_libraries(harbor-shell PRIVATE
    harbor-desktop-core Qt6::Quick Qt6::QuickControls2 Qt6::Svg LayerShellQt::Interface KF6::WindowSystem)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()

# harbor_test(<name> SOURCES <files...> [LIBS <targets...>] [TARGET <executable>] [DBUS] [QML])
#   Builds <executable> (default: <name>-test) linked with Qt6::Test and registers test <name>.
#   DBUS runs it inside a private session bus. QML embeds resources.qrc and renders offscreen.
function(harbor_test name)
    cmake_parse_arguments(T "DBUS;QML" "TARGET" "SOURCES;LIBS" ${ARGN})
    if(NOT T_TARGET)
        set(T_TARGET ${name}-test)
    endif()
    if(T_QML)
        list(APPEND T_SOURCES resources.qrc)
    endif()
    add_executable(${T_TARGET} ${T_SOURCES})
    target_link_libraries(${T_TARGET} PRIVATE ${T_LIBS} Qt6::Test)
    if(T_DBUS)
        add_test(NAME ${name} COMMAND dbus-run-session -- $<TARGET_FILE:${T_TARGET}>)
    else()
        add_test(NAME ${name} COMMAND ${T_TARGET})
    endif()
    if(T_QML)
        set_tests_properties(${name} PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
    endif()
endfunction()

set(HARBOR_QML_LIBS Qt6::Quick Qt6::QuickControls2)

# Core
harbor_test(core TARGET core-test SOURCES tests/core_test.cpp LIBS harbor-core)
harbor_test(region SOURCES tests/region_test.cpp LIBS harbor-core)

# Shell services
harbor_test(accounts DBUS SOURCES tests/accounts_test.cpp LIBS harbor-desktop-core)
harbor_test(apps SOURCES tests/apps_test.cpp LIBS harbor-desktop-core)
harbor_test(bluetooth DBUS SOURCES tests/bluetooth_test.cpp src/core/Bluetooth.cpp LIBS Qt6::Core Qt6::DBus)
target_include_directories(bluetooth-test PRIVATE src)
harbor_test(bluetooth-pairing DBUS SOURCES tests/bluetooth_pairing_test.cpp LIBS harbor-desktop-core)
harbor_test(files SOURCES tests/files_test.cpp LIBS harbor-desktop-core)
harbor_test(files-menu DBUS SOURCES tests/files_menu_test.cpp LIBS harbor-desktop-core)
harbor_test(global-menu DBUS TARGET menu-test SOURCES tests/menu_test.cpp LIBS harbor-desktop-core)
harbor_test(keyboard-indicator DBUS SOURCES tests/keyboard_indicator_test.cpp LIBS harbor-desktop-core)
harbor_test(keyboard-notify DBUS SOURCES tests/keyboard_notify_test.cpp LIBS harbor-desktop-core)
harbor_test(launch DBUS SOURCES tests/launch_test.cpp LIBS harbor-desktop-core)
harbor_test(notifications SOURCES tests/notifications_test.cpp LIBS harbor-desktop-core)
harbor_test(system-actions SOURCES tests/system_actions_test.cpp LIBS harbor-desktop-core)
harbor_test(system-state SOURCES tests/system_state_test.cpp LIBS harbor-desktop-core)

# Native settings back ends
harbor_test(accessibility DBUS SOURCES tests/accessibility_test.cpp LIBS harbor-desktop-core)
harbor_test(application-storage SOURCES tests/application_storage_test.cpp LIBS harbor-desktop-core)
harbor_test(date-time DBUS SOURCES tests/date_time_test.cpp LIBS harbor-desktop-core)
harbor_test(default-apps SOURCES tests/default_apps_test.cpp LIBS harbor-desktop-core)
harbor_test(gesture-settings DBUS SOURCES tests/gesture_settings_test.cpp LIBS harbor-desktop-core)
harbor_test(native-hardware SOURCES tests/native_hardware_test.cpp LIBS harbor-desktop-core)
harbor_test(network-settings DBUS SOURCES tests/network_settings_test.cpp LIBS harbor-desktop-core)
harbor_test(notification-preferences SOURCES tests/notification_preferences_test.cpp LIBS harbor-desktop-core)
harbor_test(pointer-settings DBUS SOURCES tests/pointer_settings_test.cpp LIBS harbor-desktop-core)
harbor_test(printer-driver DBUS SOURCES tests/printer_driver_test.cpp LIBS harbor-desktop-core)
harbor_test(printers DBUS SOURCES tests/printers_test.cpp LIBS harbor-desktop-core)
harbor_test(storage SOURCES tests/storage_test.cpp LIBS harbor-desktop-core)
harbor_test(storage-analysis SOURCES tests/storage_analysis_test.cpp LIBS harbor-desktop-core)
harbor_test(updates DBUS SOURCES tests/update_test.cpp LIBS harbor-desktop-core)

# QML user interface
harbor_test(control-ui QML SOURCES tests/control_ui_test.cpp LIBS harbor-core ${HARBOR_QML_LIBS})
harbor_test(dock-launch QML SOURCES tests/dock_launch_test.cpp LIBS harbor-core ${HARBOR_QML_LIBS})
harbor_test(files-ui QML SOURCES tests/files_ui_test.cpp LIBS harbor-desktop-core ${HARBOR_QML_LIBS})
harbor_test(focus-ui QML SOURCES tests/focus_ui_test.cpp LIBS harbor-desktop-core ${HARBOR_QML_LIBS})
harbor_test(icons QML SOURCES tests/icons_test.cpp src/core/Icons.cpp LIBS Qt6::Quick Qt6::Svg)
target_include_directories(icons-test PRIVATE src)
harbor_test(input-menu QML SOURCES tests/input_menu_test.cpp LIBS harbor-core ${HARBOR_QML_LIBS})
harbor_test(panel-menus QML SOURCES tests/panel_menus_test.cpp LIBS harbor-core ${HARBOR_QML_LIBS})
harbor_test(region-ui QML SOURCES tests/region_ui_test.cpp LIBS harbor-core ${HARBOR_QML_LIBS})
harbor_test(settings-expansion-ui QML SOURCES tests/settings_expansion_ui_test.cpp LIBS harbor-core ${HARBOR_QML_LIBS})
harbor_test(settings-ui QML SOURCES tests/settings_ui_test.cpp LIBS harbor-core ${HARBOR_QML_LIBS})

# Probes driven by the Python integration tests in scripts/validate-trixie.sh
add_executable(gesture-kwin-probe tests/gesture_kwin_probe.cpp)
target_link_libraries(gesture-kwin-probe PRIVATE harbor-desktop-core Qt6::Core Qt6::DBus)
add_executable(sandbox-gui-probe tests/sandbox_gui_probe.cpp)
target_link_libraries(sandbox-gui-probe PRIVATE Qt6::Gui)

# ---------------------------------------------------------------------------
# Installation
# ---------------------------------------------------------------------------
install(TARGETS harbor-shell RUNTIME DESTINATION bin)
install(PROGRAMS
    scripts/harbor-accessibility-agent
    scripts/harbor-autostart
    scripts/harbor-display-guard
    scripts/harbor-display-layout-guard
    scripts/harbor-doctor
    scripts/harbor-file-operation
    scripts/harbor-files
    scripts/harbor-keyboard
    scripts/harbor-locale-environment
    scripts/harbor-power-agent
    scripts/harbor-sandbox
    scripts/harbor-session
    scripts/harbor-session-client
    scripts/harbor-settings
    scripts/harbor-user-settings
    DESTINATION bin)
# Privileged helpers, reached only through the polkit actions below.
install(PROGRAMS
    scripts/harbor-charge-limit
    scripts/harbor-generate-locales
    scripts/harbor-network-import
    DESTINATION libexec)
install(FILES config/polkit/org.harbor.charge.policy config/polkit/org.harbor.locale.policy
    DESTINATION share/polkit-1/actions)

install(FILES session/harbor.desktop DESTINATION share/wayland-sessions)
configure_file(session/org.harbor.Shell.desktop.in org.harbor.Shell.desktop @ONLY)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/org.harbor.Shell.desktop
    session/org.harbor.Files.desktop
    session/org.harbor.Settings.desktop
    DESTINATION share/applications)
install(FILES config/menus/harbor-applications.menu DESTINATION /etc/xdg/menus)
install(FILES assets/icons/app.svg DESTINATION share/icons/hicolor/scalable/apps RENAME harbor.svg)
install(DIRECTORY config/kwin/harbor-shortcuts DESTINATION share/kwin/scripts)
install(FILES config/kwin/Harbor.colors DESTINATION share/color-schemes)
install(FILES config/portals/harbor-portals.conf DESTINATION share/xdg-desktop-portal)
install(FILES LICENSE README.md THIRD_PARTY_NOTICES.md DESTINATION share/doc/harbor-desktop)
install(DIRECTORY docs/ DESTINATION share/doc/harbor-desktop/docs)

# ---------------------------------------------------------------------------
# Debian package
# ---------------------------------------------------------------------------
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME harbor-desktop)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "Harbor Desktop contributors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Independent Qt desktop shell and settings for KWin Wayland")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Harbor Desktop contributors")
set(CPACK_DEBIAN_PACKAGE_SECTION x11)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(HARBOR_DEPENDS
    python3 locales pkexec xkb-data libglib2.0-bin xwayland kwin-wayland kwin-style-breeze
    dbus-user-session qt6-wayland qt6-svg-plugins qml6-module-qtquick qml6-module-qtquick-window
    qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtquick-templates
    qml6-module-qtqml-workerscript)
set(HARBOR_RECOMMENDS
    dolphin konsole xdg-utils breeze-cursor-theme accountsservice libcrypt1 packagekit bubblewrap
    avahi-daemon cups cups-pk-helper pulseaudio-utils ddcutil swayidle orca at-spi2-core
    network-manager network-manager-gnome network-manager-openvpn-gnome python3-gi gir1.2-nm-1.0
    wireplumber pipewire pipewire-pulse bluez blueman pavucontrol cups-client brightnessctl
    power-profiles-daemon kscreen polkit-kde-agent-1 xdg-desktop-portal-kde xdg-desktop-portal-gtk
    xwayland fonts-noto-core)
list(JOIN HARBOR_DEPENDS ", " CPACK_DEBIAN_PACKAGE_DEPENDS)
list(JOIN HARBOR_RECOMMENDS ", " CPACK_DEBIAN_PACKAGE_RECOMMENDS)
include(CPack)
