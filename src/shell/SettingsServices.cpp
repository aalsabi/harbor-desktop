#include "SettingsServices.h"
#include <QQmlContext>

void SettingsServices::expose(QQmlContext* context) {
    context->setContextProperty("ApplicationStorage", &applicationStorage);
    context->setContextProperty("PrinterDrivers", &printerDrivers);
    context->setContextProperty("GestureSettings", &gestures);
    context->setContextProperty("NotificationPrefs", &notificationPreferences);
    context->setContextProperty("UserSettings", &user);
    context->setContextProperty("AccessibilitySettings", &accessibility);
    context->setContextProperty("NetworkSettings", &network);
    context->setContextProperty("DisplaySettings", &display);
    context->setContextProperty("AudioStreams", &audio);
    context->setContextProperty("PowerSettings", &power);
    context->setContextProperty("SoftwareUpdate", &softwareUpdate);
    context->setContextProperty("DateTimeSettings", &dateTime);
    context->setContextProperty("StorageSettings", &storage);
    context->setContextProperty("PointerSettings", &pointer);
    context->setContextProperty("DefaultApps", &defaultApps);
    context->setContextProperty("Printers", &printers);
}
