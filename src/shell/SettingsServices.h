#pragma once
#include "core/AccessibilitySettings.h"
#include "core/ApplicationStorage.h"
#include "core/AudioStreams.h"
#include "core/DateTimeSettings.h"
#include "core/DefaultApps.h"
#include "core/DisplaySettings.h"
#include "core/GestureSettings.h"
#include "core/NetworkSettings.h"
#include "core/NotificationPreferences.h"
#include "core/PointerSettings.h"
#include "core/PowerSettings.h"
#include "core/PrinterDrivers.h"
#include "core/Printers.h"
#include "core/SoftwareUpdate.h"
#include "core/StorageSettings.h"
#include "core/UserSettings.h"
class QQmlContext;

// Back ends of the native System Settings pages. Each queries its service only when a page uses it.
// Members are constructed in declaration order and destroyed in reverse.
struct SettingsServices {
    DateTimeSettings dateTime;
    StorageSettings storage;
    PointerSettings pointer;
    DefaultApps defaultApps;
    Printers printers;
    NotificationPreferences notificationPreferences;
    UserSettings user;
    AccessibilitySettings accessibility;
    NetworkSettings network;
    DisplaySettings display;
    AudioStreams audio;
    PowerSettings power;
    SoftwareUpdate softwareUpdate;
    PrinterDrivers printerDrivers;
    ApplicationStorage applicationStorage;
    GestureSettings gestures;

    // Registers each back end under the name the QML pages use.
    void expose(QQmlContext* context);
};
