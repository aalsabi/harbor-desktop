import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    radius: 16
    clip: true
    color: Prefs.dark ? "#242426" : "#f5f5f7"
    LayoutMirroring.enabled: Prefs.language === "ar"
    LayoutMirroring.childrenInherit: true
    property real previousOpacity: Prefs.opacity < 1 ? Prefs.opacity : .9
    property string section: "General"
    property var pages: [
        {
            key: QT_TR_NOOP("Wi-Fi"),
            icon: "◔",
            color: Prefs.accent,
            id: "wifi",
            group: 0,
            tags: "wireless internet شبكة"
        },
        {
            key: QT_TR_NOOP("Bluetooth"),
            icon: "ᛒ",
            color: Prefs.accent,
            id: "bluetooth",
            group: 0,
            tags: "devices أجهزة"
        },
        {
            key: QT_TR_NOOP("Network"),
            icon: "◎",
            color: "#2488d8",
            id: "network",
            group: 0,
            tags: "internet ethernet"
        },
        {
            key: QT_TR_NOOP("Notifications & Focus"),
            icon: "●",
            color: "#ed4b66",
            id: "notifications",
            group: 1,
            tags: "do not disturb focus تنبيهات عدم الإزعاج"
        },
        {
            key: QT_TR_NOOP("Sound"),
            icon: "♪",
            color: "#ed4b66",
            id: "sound",
            group: 1,
            tags: "volume audio صوت"
        },
        {
            key: QT_TR_NOOP("General"),
            icon: "⚙",
            color: "#8a8b90",
            id: "general",
            group: 2,
            tags: "about updates language تحديث لغة"
        },
        {
            key: QT_TR_NOOP("Language & Region"),
            icon: "◎",
            color: "#348dda",
            id: "region",
            group: 2,
            tags: "locale dates time currency languages لغة منطقة تاريخ عملة"
        },
        {
            key: QT_TR_NOOP("Date & Time"),
            icon: "◷",
            color: "#388fca",
            id: "datetime",
            group: 2,
            tags: "clock timezone automatic ntp ساعة تاريخ وقت"
        },
        {
            key: QT_TR_NOOP("Default Applications"),
            icon: "▦",
            color: "#7383ce",
            id: "defaults",
            group: 2,
            tags: "browser email pdf files default متصفح بريد افتراضي"
        },
        {
            key: QT_TR_NOOP("Storage"),
            icon: "▤",
            color: "#838b98",
            id: "storage",
            group: 2,
            tags: "disk drive space volume mount قرص مساحة تخزين"
        },
        {
            key: QT_TR_NOOP("Gestures"),
            icon: "✥",
            color: "#488daa",
            id: "gestures",
            group: 2,
            tags: "touchpad swipe gestures إيماءات سحب"
        },
        {
            key: QT_TR_NOOP("Startup Applications"),
            icon: "▷",
            color: "#488daa",
            id: "startup",
            group: 2,
            tags: "login apps startup دخول تشغيل"
        },
        {
            key: QT_TR_NOOP("Privacy & Permissions"),
            icon: "◈",
            color: "#658e78",
            id: "privacy",
            group: 2,
            tags: "flatpak permissions privacy خصوصية صلاحيات"
        },
        {
            key: QT_TR_NOOP("Appearance"),
            icon: "◐",
            color: "#77777e",
            id: "appearance",
            group: 2,
            tags: "light dark theme مظهر"
        },
        {
            key: QT_TR_NOOP("Accessibility"),
            icon: "◎",
            color: "#258de9",
            id: "accessibility",
            group: 2,
            tags: "motion movement حركة"
        },
        {
            key: QT_TR_NOOP("Desktop & Dock"),
            icon: "▣",
            color: "#437ee9",
            id: "desktop",
            group: 3,
            tags: "panel glass transparency شفافية"
        },
        {
            key: QT_TR_NOOP("Displays"),
            icon: "▱",
            color: "#7274e5",
            id: "displays",
            group: 3,
            tags: "resolution scale brightness دقة سطوع"
        },
        {
            key: QT_TR_NOOP("Keyboard"),
            icon: "⌨",
            color: "#85858d",
            id: "keyboard",
            group: 3,
            tags: "input language arabic english shortcut كيبورد عربي انجليزي لغة"
        },
        {
            key: QT_TR_NOOP("Mouse"),
            icon: "◉",
            color: "#8c8d97",
            id: "mouse",
            group: 3,
            tags: "pointer speed natural scroll left handed فأرة مؤشر تمرير"
        },
        {
            key: QT_TR_NOOP("Touchpad"),
            icon: "▱",
            color: "#8c8d97",
            id: "touchpad",
            group: 3,
            tags: "trackpad tap click natural scroll touch لمس نقر تمرير"
        },
        {
            key: QT_TR_NOOP("Printers"),
            icon: "▣",
            color: "#678dac",
            id: "printers",
            group: 3,
            tags: "printing cups queue طابعة طباعة"
        },
        {
            key: QT_TR_NOOP("Battery"),
            icon: "▰",
            color: "#39a958",
            id: "battery",
            group: 3,
            tags: "power performance energy طاقة"
        },
        {
            key: QT_TR_NOOP("Users & Groups"),
            icon: "♙",
            color: "#5a83ce",
            id: "users",
            group: 4,
            tags: "account name حساب"
        }
    ]
    property var nativePages: ({
            "Gestures": "GesturesPage.qml",
            "Wi-Fi": "NetworkPage.qml",
            "Network": "NetworkPage.qml",
            "Bluetooth": "BluetoothPage.qml",
            "Users & Groups": "AccountsPage.qml",
            "Software Update": "UpdatesPage.qml",
            "Notifications & Focus": "NotificationsPage.qml",
            "Startup Applications": "StartupPage.qml",
            "Privacy & Permissions": "PrivacyPage.qml",
            "Date & Time": "DateTimePage.qml",
            "Storage": "StoragePage.qml",
            "Default Applications": "DefaultAppsPage.qml",
            "Mouse": "PointerPage.qml",
            "Touchpad": "PointerPage.qml",
            "Printers": "PrintersPage.qml"
        })
    property var results: pages.filter(p => (p.key + " " + qsTr(p.key) + " " + p.tags).toLowerCase().includes(search.text.toLowerCase()))
    property var currentAccount: Accounts.users.find(u => u.UserName === System.state.userName) || null
    // About and Software Update are reached from General rather than the sidebar; list their
    // titles so lupdate extracts them for the page header.
    readonly property var linkedPageTitles: [QT_TR_NOOP("About"), QT_TR_NOOP("Software Update")]
    property var currentPage: pages.find(p => p.key === section) || ({
            key: section,
            icon: "⚙",
            color: "#8a8b90"
        })
    onSectionChanged: pageScroll.contentItem.contentY = 0
    RowLayout {
        anchors.fill: parent
        spacing: 0
        Rectangle {
            Layout.preferredWidth: 238
            Layout.fillHeight: true
            color: Prefs.dark ? "#2e2e33" : "#e9e9ef"
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    MouseArea {
                        anchors.fill: parent
                        onPressed: UI.windowAction("move")
                    }
                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 0
                        Repeater {
                            model: ["#ff6057", "#febc2e", "#28c840"]
                            delegate: Button {
                                required property string modelData
                                required property int index
                                width: 24
                                height: 26
                                Accessible.name: ["Close window", "Minimize window", "Maximize window"][index]
                                contentItem: Item {}
                                background: Item {
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 12
                                        height: 12
                                        radius: 6
                                        color: parent.parent.modelData
                                        border.width: parent.parent.activeFocus ? 2 : 1
                                        border.color: parent.parent.activeFocus ? Prefs.accent : Qt.darker(color, 1.12)
                                    }
                                }
                                onClicked: UI.windowAction(index === 0 ? "close" : index === 1 ? "minimize" : "maximize")
                            }
                        }
                    }
                }
                HarborField {
                    id: search
                    objectName: "settings-search"
                    Layout.fillWidth: true
                    implicitHeight: 30
                    placeholderText: qsTr("Search")
                    font.pixelSize: 13
                }
                Button {
                    Layout.fillWidth: true
                    implicitHeight: 58
                    onClicked: root.section = "Users & Groups"
                    background: Rectangle {
                        color: parent.hovered ? (Prefs.dark ? "#414148" : "#dedee6") : "transparent"
                        radius: 8
                    }
                    contentItem: RowLayout {
                        spacing: 10
                        Rectangle {
                            width: 40
                            height: 40
                            radius: 20
                            color: "#a0a6b5"
                            Text {
                                anchors.centerIn: parent
                                text: "♙"
                                font.pixelSize: 28
                                color: "white"
                            }
                        }
                        ColumnLayout {
                            spacing: 2
                            SettingsLabel {
                                text: root.currentAccount ? (root.currentAccount.RealName || root.currentAccount.UserName) : qsTr("Local account")
                                font.bold: true
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                            }
                            SettingsNote {
                                text: qsTr("Account settings")
                            }
                        }
                    }
                }
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: availableWidth
                    clip: true
                    ColumnLayout {
                        width: parent.width
                        spacing: 3
                        Repeater {
                            model: root.results
                            delegate: Button {
                                required property var modelData
                                required property int index
                                objectName: "settings-nav-" + modelData.id
                                Layout.fillWidth: true
                                Layout.preferredHeight: 33
                                Layout.topMargin: index > 0 && root.results[index - 1].group !== modelData.group ? 9 : 0
                                Accessible.name: qsTr(modelData.key)
                                onClicked: root.section = modelData.key
                                background: Rectangle {
                                    radius: 6
                                    color: root.section === parent.modelData.key ? Prefs.accent : parent.hovered ? (Prefs.dark ? "#414148" : "#dcdce5") : "transparent"
                                }
                                contentItem: RowLayout {
                                    spacing: 9
                                    Rectangle {
                                        width: 25
                                        height: 25
                                        radius: 6
                                        color: modelData.color
                                        Text {
                                            anchors.centerIn: parent
                                            text: modelData.icon
                                            color: "white"
                                            font.pixelSize: 19
                                        }
                                    }
                                    Text {
                                        text: qsTr(modelData.key)
                                        color: root.section === modelData.key ? "white" : SettingsTheme.ink
                                        font.pixelSize: 13
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                        SettingsNote {
                            visible: root.results.length === 0
                            text: qsTr("No matching settings")
                        }
                    }
                }
            }
        }
        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: SettingsTheme.line
        }
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                MouseArea {
                    anchors.fill: parent
                    onPressed: UI.windowAction("move")
                    onDoubleClicked: UI.windowAction("maximize")
                }
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 24
                    anchors.rightMargin: 24
                    HarborButton {
                        text: "‹"
                        visible: root.section === "About" || root.section === "Software Update"
                        onClicked: root.section = "General"
                        Accessible.name: "Back to General"
                    }
                    SettingsLabel {
                        text: qsTr(root.currentPage.key)
                        font.bold: true
                        font.pixelSize: 20
                    }
                }
            }
            ScrollView {
                id: pageScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: availableWidth
                clip: true
                ColumnLayout {
                    width: pageScroll.availableWidth
                    spacing: 18
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 24
                        Layout.rightMargin: 24
                        Layout.bottomMargin: 28
                        spacing: 18
                        Loader {
                            id: nativePage
                            objectName: "native-settings-page"
                            Layout.fillWidth: true
                            Layout.preferredHeight: item ? item.implicitHeight : 0
                            active: !!root.nativePages[root.section]
                            visible: active
                            source: active ? root.nativePages[root.section] : ""
                            onLoaded: if (root.section === "Mouse" || root.section === "Touchpad")
                                item.deviceType = root.section === "Mouse" ? "mouse" : "touchpad"
                            Connections {
                                target: root
                                function onSectionChanged() {
                                    if (nativePage.item && typeof nativePage.item.deviceType !== "undefined" && (root.section === "Mouse" || root.section === "Touchpad"))
                                        nativePage.item.deviceType = root.section === "Mouse" ? "mouse" : "touchpad";
                                }
                            }
                        }
                        GeneralSection {
                            visible: root.section === "General"
                            settings: root
                        }
                        Loader {
                            Layout.fillWidth: true
                            active: root.section === "Language & Region"
                            visible: active
                            source: active ? "LanguageRegion.qml" : ""
                            onLoaded: item.keyboardRequested.connect(function () {
                                root.section = "Keyboard";
                            })
                        }
                        AppearanceSection {
                            visible: root.section === "Appearance"
                            settings: root
                        }
                        SoundSection {
                            visible: root.section === "Sound"
                            settings: root
                        }
                        AccessibilitySection {
                            visible: root.section === "Accessibility"
                            settings: root
                        }
                        DesktopDockSection {
                            visible: root.section === "Desktop & Dock"
                            settings: root
                        }
                        DisplaysSection {
                            visible: root.section === "Displays"
                            settings: root
                        }
                        KeyboardSection {
                            visible: root.section === "Keyboard"
                            settings: root
                        }
                        BatterySection {
                            visible: root.section === "Battery"
                            settings: root
                        }
                        AboutSection {
                            visible: root.section === "About"
                            settings: root
                        }
                        SettingsNote {
                            text: System.message
                            visible: text.length > 0
                            color: Prefs.dark ? "#e0b471" : "#986318"
                        }
                    }
                }
            }
        }
    }
    MouseArea {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: 16
        height: 16
        cursorShape: Qt.SizeFDiagCursor
        onPressed: UI.windowAction("resize")
    }
}
