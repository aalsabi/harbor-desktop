import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GlassCard {
    id: root
    radius: 12
    focus: true
    Keys.onEscapePressed: UI.dismiss()
    property string pending: ""
    property string caption: ""
    function ask(action, label) {
        pending = action;
        caption = label;
        confirm.open();
    }
    component Entry: HarborButton {
        Layout.fillWidth: true
        implicitHeight: 32
        background: Rectangle {
            radius: 5
            color: parent.hovered ? (Prefs.dark ? "#405b80" : "#d2e4ff") : "transparent"
        }
        contentItem: HarborLabel {
            text: parent.text
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
    component Divider: Rectangle {
        Layout.fillWidth: true
        implicitHeight: 1
        color: Prefs.dark ? "#40ffffff" : "#20000000"
    }
    Component.onCompleted: System.refreshRecent()
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 3
        Entry {
            objectName: "harbor-about"
            text: qsTr("About This Computer")
            onClicked: UI.open("settings:About")
        }
        Divider {}
        Entry {
            text: qsTr("System Settings…")
            onClicked: UI.open("settings")
        }
        Entry {
            text: qsTr("Software Center")
            onClicked: System.openTool("updates")
        }
        Divider {}
        Entry {
            text: qsTr("Recent Items") + "  ›"
            onClicked: UI.open("recent")
        }
        Divider {}
        Entry {
            text: qsTr("Force Quit…")
            onClicked: root.ask("force-quit", text)
        }
        Divider {}
        Entry {
            text: qsTr("Sleep")
            enabled: !!System.state.canSuspend
            onClicked: System.action("sleep")
        }
        Entry {
            text: qsTr("Restart…")
            enabled: !!System.state.canReboot
            onClicked: root.ask("reboot", text)
        }
        Entry {
            text: qsTr("Shut Down…")
            enabled: !!System.state.canPowerOff
            onClicked: root.ask("poweroff", text)
        }
        Divider {}
        Entry {
            text: qsTr("Lock Screen")
            enabled: !!System.state.lockAvailable
            onClicked: System.action("lock")
        }
        Entry {
            objectName: "harbor-logout"
            text: qsTr("Log Out ") + (System.state.userDisplayName || System.state.userName || "Abdullah").split(" ")[0] + "…"
            enabled: !!System.state.harborSession
            onClicked: root.ask("logout", text)
        }
        HarborLabel {
            text: System.message || ""
            visible: text !== ""
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            font.pixelSize: 11
        }
        Item {
            Layout.fillHeight: true
        }
    }
    Dialog {
        id: confirm
        objectName: "harbor-confirm"
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(340, root.width - 20)
        modal: true
        title: root.caption
        standardButtons: Dialog.Ok | Dialog.Cancel
        contentItem: HarborLabel {
            wrapMode: Text.Wrap
            text: root.pending === "force-quit" ? qsTr("Select a window after confirming. Unsaved work in that application will be lost. Press Escape to cancel selection.") : qsTr("Save your work before continuing.")
        }
        onAccepted: {
            if (root.pending === "logout")
                UI.logout();
            else
                System.action(root.pending);
        }
    }
}
