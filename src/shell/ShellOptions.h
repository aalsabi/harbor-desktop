#pragma once
#include <QSize>
#include <QString>
#include <QStringList>

// Command-line options of harbor-shell. Without a mode flag the process runs the desktop session;
// the flags can be combined (for example --files with --preview) and keep their historic precedence.
struct ShellOptions {
    bool preview = false;
    bool settings = false;
    bool files = false;
    bool control = false;
    bool diagnose = false;
    bool screenshot = false;               // --screenshot <path> was given
    QString controlPage = "ControlCenter"; // --control [HarborMenu|BluetoothMenu|RecentItems]
    QString initialPath;                   // --files <path> or --preview <path>
    QString settingsPage;                  // --settings <page>, only when it names a known page
    QString screenshotPath;                // --screenshot <path>
    QSize screenshotSize;                  // --screenshot-size <W>x<H>, only within the supported range

    static ShellOptions parse(const QStringList& args);
    bool session() const { return !preview && !settings && !files && !control; }
    // The single window shown outside the session, and its initial size.
    QString windowName() const;
    QSize windowSize() const;
};
