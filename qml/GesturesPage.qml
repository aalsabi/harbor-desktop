pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    spacing: 16
    property color ink: Prefs.dark ? "#eeeef0" : "#252527"
    property color muted: Prefs.dark ? "#a6a6ad" : "#76767c"
    property var draft: ({})
    property bool draftEnabled: false
    property bool hasTouchpad: PointerSettings.devices.some(d => d.touchpad && d.gestureSupport)
    property var actions: [
        {
            key: "none",
            en: QT_TR_NOOP("No custom action")
        },
        {
            key: "launcher",
            en: QT_TR_NOOP("Application launcher")
        },
        {
            key: "control",
            en: QT_TR_NOOP("Control Center")
        },
        {
            key: "notifications",
            en: QT_TR_NOOP("Notifications")
        },
        {
            key: "settings",
            en: QT_TR_NOOP("Settings")
        },
        {
            key: "windows",
            en: QT_TR_NOOP("Window list")
        }
    ]
    property var rows: [
        {
            key: "3-Up",
            en: QT_TR_NOOP("Three fingers up")
        },
        {
            key: "3-Down",
            en: QT_TR_NOOP("Three fingers down")
        },
        {
            key: "3-Left",
            en: QT_TR_NOOP("Three fingers left")
        },
        {
            key: "3-Right",
            en: QT_TR_NOOP("Three fingers right")
        },
        {
            key: "4-Up",
            en: QT_TR_NOOP("Four fingers up")
        },
        {
            key: "4-Down",
            en: QT_TR_NOOP("Four fingers down")
        },
        {
            key: "4-Left",
            en: QT_TR_NOOP("Four fingers left")
        },
        {
            key: "4-Right",
            en: QT_TR_NOOP("Four fingers right")
        }
    ]
    function reload() {
        draft = Object.assign({}, GestureSettings.mappings);
        draftEnabled = GestureSettings.enabled;
    }
    Component.onCompleted: {
        reload();
        GestureSettings.refresh();
        PointerSettings.setActive(true);
    }
    Component.onDestruction: PointerSettings.setActive(false)
    Connections {
        target: GestureSettings
        function onChanged() {
            if (!GestureSettings.busy)
                root.reload();
        }
    }
    Text {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: root.muted
        text: qsTr("Custom touchpad swipes are off by default. KWin's built-in gestures may also run for the same swipe. Use only the actions you want and disable custom gestures if they conflict.")
    }
    Text {
        Layout.fillWidth: true
        visible: !root.hasTouchpad
        wrapMode: Text.WordWrap
        color: root.muted
        text: qsTr("No gesture-capable touchpad is currently detected. Saved mappings will work when a compatible touchpad is connected.")
    }
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: body.implicitHeight + 28
        radius: 10
        color: Prefs.dark ? "#303034" : "white"
        border.color: Prefs.dark ? "#454549" : "#e2e2e7"
        ColumnLayout {
            id: body
            anchors.fill: parent
            anchors.margins: 14
            spacing: 10
            Switch {
                text: qsTr("Enable custom gestures")
                checked: root.draftEnabled
                enabled: GestureSettings.available && !GestureSettings.busy
                onClicked: root.draftEnabled = checked
            }
            Repeater {
                model: root.rows
                delegate: RowLayout {
                    id: row
                    required property var modelData
                    Layout.fillWidth: true
                    Text {
                        Layout.fillWidth: true
                        color: root.ink
                        text: qsTr(row.modelData.en)
                        wrapMode: Text.WordWrap
                    }
                    ComboBox {
                        Layout.preferredWidth: 190
                        model: root.actions.map(a => qsTr(a.en))
                        currentIndex: Math.max(0, root.actions.findIndex(a => a.key === (root.draft[row.modelData.key] || "none")))
                        enabled: !GestureSettings.busy
                        onActivated: {
                            let copy = Object.assign({}, root.draft);
                            copy[row.modelData.key] = root.actions[currentIndex].key;
                            root.draft = copy;
                        }
                    }
                }
            }
        }
    }
    Text {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: root.muted
        text: GestureSettings.error || (GestureSettings.active ? qsTr("Custom gesture handlers are active.") : qsTr("Custom gesture handlers are not active."))
    }
    RowLayout {
        HarborButton {
            text: qsTr("Apply gestures")
            enabled: GestureSettings.available && !GestureSettings.busy
            onClicked: GestureSettings.apply(root.draft, root.draftEnabled)
        }
        HarborButton {
            text: qsTr("Reload")
            enabled: !GestureSettings.busy
            onClicked: GestureSettings.refresh()
        }
    }
}
