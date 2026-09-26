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
    property color ink: Prefs.dark ? "#eeeef0" : "#252527"
    property color muted: Prefs.dark ? "#a6a6ad" : "#76767c"
    property color card: Prefs.dark ? "#303034" : "#ffffff"
    property color line: Prefs.dark ? "#454549" : "#e2e2e7"
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
    property var currentPage: pages.find(p => p.key === section) || ({
            key: section,
            icon: "⚙",
            color: "#8a8b90"
        })
    onSectionChanged: pageScroll.contentItem.contentY = 0
    component Label: Text {
        color: root.ink
        font.pixelSize: 13
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
    }
    component Note: Label {
        color: root.muted
        font.pixelSize: 12
        lineHeight: 1.2
    }
    component Group: Rectangle {
        default property alias contents: groupBody.data
        Layout.fillWidth: true
        implicitHeight: groupBody.implicitHeight + 28
        radius: 10
        color: root.card
        border.color: root.line
        ColumnLayout {
            id: groupBody
            anchors.fill: parent
            anchors.margins: 14
            spacing: 12
        }
    }
    component Divider: Rectangle {
        Layout.fillWidth: true
        height: 1
        color: root.line
    }
    component SettingRow: RowLayout {
        property string label
        property string hint: ""
        default property alias controls: controlRow.data
        Layout.fillWidth: true
        spacing: 18
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Label {
                text: parent.parent.label
            }
            Note {
                text: parent.parent.hint
                visible: text.length > 0
            }
        }
        RowLayout {
            id: controlRow
            spacing: 8
        }
    }
    component LinkRow: Button {
        property string destination
        Layout.fillWidth: true
        implicitHeight: 39
        text: destination
        onClicked: root.section = destination
        background: Rectangle {
            radius: 6
            color: parent.hovered ? (Prefs.dark ? "#45454b" : "#f0f0f5") : "transparent"
        }
        contentItem: RowLayout {
            Label {
                text: parent.parent.text
            }
            Text {
                text: Prefs.language === "ar" ? "‹" : "›"
                font.pixelSize: 23
                color: root.muted
            }
        }
    }
    component Select: ComboBox {
        id: select
        implicitHeight: 30
        implicitWidth: 170
        font.pixelSize: 13
        leftPadding: 10
        rightPadding: 27
        contentItem: Text {
            text: select.displayText
            color: root.ink
            font: select.font
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            radius: 6
            color: Prefs.dark ? "#45454b" : "#fafafa"
            border.color: select.activeFocus ? Prefs.accent : root.line
            border.width: select.activeFocus ? 2 : 1
        }
        indicator: Text {
            x: select.width - width - 9
            y: (select.height - height) / 2
            text: "⌄"
            color: root.muted
            font.pixelSize: 16
        }
    }
    component ThemeChoice: Button {
        property bool night: false
        implicitWidth: 154
        implicitHeight: 105
        onClicked: Prefs.dark = night
        Accessible.name: night ? "Dark appearance" : "Light appearance"
        background: Rectangle {
            radius: 9
            color: parent.night ? "#22222b" : "#e0eaff"
            border.width: Prefs.dark === parent.night ? 3 : 1
            border.color: Prefs.dark === parent.night ? Prefs.accent : root.line
            Rectangle {
                anchors.centerIn: parent
                width: 112
                height: 72
                radius: 6
                color: parent.parent.night ? "#36363d" : "#ffffff"
                Rectangle {
                    width: 30
                    height: parent.height
                    radius: 6
                    color: parent.parent.parent.night ? "#4a4a52" : "#e8e8ef"
                }
                Column {
                    anchors.centerIn: parent
                    anchors.horizontalCenterOffset: 12
                    spacing: 7
                    Repeater {
                        model: 3
                        Rectangle {
                            width: 54
                            height: 5
                            radius: 2
                            color: night ? "#60606d" : "#d9d9e2"
                        }
                    }
                }
            }
        }
        contentItem: Item {}
    }
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
                            Label {
                                text: root.currentAccount ? (root.currentAccount.RealName || root.currentAccount.UserName) : qsTr("Local account")
                                font.bold: true
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                            }
                            Note {
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
                                        color: root.section === modelData.key ? "white" : root.ink
                                        font.pixelSize: 13
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                        Note {
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
            color: root.line
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
                    Label {
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
                        ColumnLayout {
                            visible: root.section === "General"
                            Layout.fillWidth: true
                            spacing: 18
                            Rectangle {
                                Layout.alignment: Qt.AlignHCenter
                                width: 64
                                height: 64
                                radius: 15
                                color: "#888a93"
                                Text {
                                    anchors.centerIn: parent
                                    text: "⚙"
                                    font.pixelSize: 49
                                    color: "white"
                                }
                            }
                            Label {
                                text: qsTr("General")
                                font.pixelSize: 24
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                            }
                            Note {
                                text: qsTr("Manage your desktop, language and system information.")
                                horizontalAlignment: Text.AlignHCenter
                            }
                            Group {
                                LinkRow {
                                    destination: "About"
                                    text: qsTr("About")
                                }
                                Divider {}
                                LinkRow {
                                    destination: "Software Update"
                                    text: qsTr("Software Update")
                                }
                            }
                            Group {
                                LinkRow {
                                    destination: "Date & Time"
                                    text: qsTr("Date & Time")
                                }
                                Divider {}
                                LinkRow {
                                    destination: "Storage"
                                    text: qsTr("Storage")
                                }
                                Divider {}
                                LinkRow {
                                    destination: "Default Applications"
                                    text: qsTr("Default Applications")
                                }
                            }
                            Group {
                                LinkRow {
                                    destination: "Language & Region"
                                    text: qsTr("Language & Region")
                                }
                                Divider {}
                                LinkRow {
                                    destination: "Keyboard"
                                    text: qsTr("Keyboard input sources")
                                }
                            }
                            Group {
                                Label {
                                    text: qsTr("Session")
                                    font.bold: true
                                }
                                SessionActions {}
                                Note {
                                    text: qsTr("Save your work before signing out or powering off.")
                                }
                            }
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
                        ColumnLayout {
                            visible: root.section === "Appearance"
                            Layout.fillWidth: true
                            spacing: 18
                            Group {
                                Label {
                                    text: qsTr("Appearance")
                                    font.bold: true
                                }
                                RowLayout {
                                    Layout.alignment: Qt.AlignHCenter
                                    spacing: 22
                                    ColumnLayout {
                                        ThemeChoice {
                                            night: false
                                            objectName: "appearance-light"
                                        }
                                        Label {
                                            text: qsTr("Light")
                                            horizontalAlignment: Text.AlignHCenter
                                        }
                                    }
                                    ColumnLayout {
                                        ThemeChoice {
                                            night: true
                                            objectName: "appearance-dark"
                                        }
                                        Label {
                                            text: qsTr("Dark")
                                            horizontalAlignment: Text.AlignHCenter
                                        }
                                    }
                                }
                            }
                            Group {
                                SettingRow {
                                    label: qsTr("Accent colour")
                                    Row {
                                        spacing: 8
                                        Repeater {
                                            model: ["#1684f8", "#168044", "#7955c9", "#b65b00", "#c63f75"]
                                            delegate: Button {
                                                required property string modelData
                                                width: 28
                                                height: 28
                                                Accessible.name: modelData
                                                contentItem: Item {}
                                                background: Rectangle {
                                                    radius: 14
                                                    color: parent.modelData
                                                    border.width: Prefs.accent === parent.modelData ? 3 : 0
                                                    border.color: root.ink
                                                }
                                                onClicked: Prefs.accent = modelData
                                            }
                                        }
                                    }
                                }
                            }
                            Note {
                                text: qsTr("Appearance applies to Harbor windows and desktop panels. Other apps use their own themes.")
                            }
                        }
                        ColumnLayout {
                            visible: root.section === "Sound"
                            Layout.fillWidth: true
                            spacing: 18
                            Loader {
                                active: root.section === "Sound"
                                Layout.fillWidth: true
                                Layout.preferredHeight: item ? item.implicitHeight : 0
                                source: active ? "AudioStreamsPage.qml" : ""
                            }
                            Group {
                                Label {
                                    text: qsTr("Output")
                                    font.bold: true
                                }
                                SettingRow {
                                    label: qsTr("Output volume")
                                    Label {
                                        Layout.preferredWidth: 90
                                        text: Math.round((parseFloat((System.state.volume || "Volume: 0").split(" ")[1]) || 0) * 100) + "%"
                                    }
                                }
                                Slider {
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 1
                                    value: parseFloat((System.state.volume || "Volume: 0").split(" ")[1]) || 0
                                    enabled: !!System.state.volumeAvailable
                                    onMoved: {
                                        audioTimer.requestedValue = value;
                                        audioTimer.restart();
                                    }
                                    Timer {
                                        id: audioTimer
                                        property real requestedValue: 0
                                        interval: 180
                                        onTriggered: System.action("volume", requestedValue)
                                    }
                                }
                                SettingRow {
                                    label: qsTr("Mute")
                                    Switch {
                                        checked: (System.state.volume || "").includes("MUTED")
                                        enabled: !!System.state.volumeAvailable && !System.busy
                                        onToggled: System.action("mute")
                                    }
                                }
                            }
                            Group {
                                SettingRow {
                                    label: qsTr("Output device")
                                    Select {
                                        objectName: "audio-output"
                                        Layout.preferredWidth: 260
                                        model: System.state.audioOutputs || []
                                        textRole: "name"
                                        currentIndex: (System.state.audioOutputs || []).findIndex(x => x.id === System.state.defaultOutputId)
                                        enabled: count > 0 && !System.busy
                                        onActivated: if (currentIndex >= 0)
                                            System.action("audio-output", model[currentIndex].id)
                                    }
                                }
                                Divider {}
                                SettingRow {
                                    label: qsTr("Input device")
                                    Select {
                                        objectName: "audio-input"
                                        Layout.preferredWidth: 260
                                        model: System.state.audioInputs || []
                                        textRole: "name"
                                        currentIndex: (System.state.audioInputs || []).findIndex(x => x.id === System.state.defaultInputId)
                                        enabled: count > 0 && !System.busy
                                        onActivated: if (currentIndex >= 0)
                                            System.action("audio-input", model[currentIndex].id)
                                    }
                                }
                                Label {
                                    text: qsTr("Input volume")
                                    font.bold: true
                                }
                                Slider {
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 1
                                    value: Number(System.state.inputVolume) || 0
                                    enabled: !!System.state.inputVolumeAvailable
                                    onMoved: {
                                        inputTimer.requestedValue = value;
                                        inputTimer.restart();
                                    }
                                    Timer {
                                        id: inputTimer
                                        property real requestedValue: 0
                                        interval: 180
                                        onTriggered: System.action("input-volume", requestedValue)
                                    }
                                }
                                SettingRow {
                                    label: qsTr("Mute microphone")
                                    Switch {
                                        checked: !!System.state.inputMuted
                                        enabled: !!System.state.inputVolumeAvailable && !System.busy
                                        onToggled: System.action("input-mute")
                                    }
                                }
                            }
                        }
                        ColumnLayout {
                            visible: root.section === "Accessibility"
                            Layout.fillWidth: true
                            spacing: 18
                            Loader {
                                active: root.section === "Accessibility"
                                Layout.fillWidth: true
                                Layout.preferredHeight: item ? item.implicitHeight : 0
                                source: active ? "AccessibilityPage.qml" : ""
                            }
                            Group {
                                SettingRow {
                                    label: qsTr("Reduce motion")
                                    hint: qsTr("Reduce animation in Harbor")
                                    Switch {
                                        checked: Prefs.reduceMotion
                                        onToggled: Prefs.reduceMotion = checked
                                    }
                                }
                            }
                            Group {
                                SettingRow {
                                    label: qsTr("Reduce transparency")
                                    Switch {
                                        checked: Prefs.opacity === 1
                                        onToggled: {
                                            if (checked) {
                                                root.previousOpacity = Prefs.opacity;
                                                Prefs.opacity = 1;
                                            } else
                                                Prefs.opacity = root.previousOpacity;
                                        }
                                    }
                                }
                            }
                            Note {
                                text: qsTr("These controls apply to Harbor. Screen-reader, magnifier and assistive-input configuration is not included yet.")
                            }
                        }
                        ColumnLayout {
                            visible: root.section === "Desktop & Dock"
                            Layout.fillWidth: true
                            spacing: 18
                            Group {
                                Image {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 170
                                    source: "qrc:/assets/wallpapers/" + Prefs.wallpaper + ".svg"
                                    fillMode: Image.PreserveAspectCrop
                                    clip: true
                                }
                                Note {
                                    text: qsTr("Harbor • Original artwork")
                                }
                            }
                            Group {
                                SettingRow {
                                    label: qsTr("Wallpaper")
                                    Select {
                                        model: ["Harbor", "Sunset", "Forest"]
                                        property var keys: ["harbor", "sunset", "forest"]
                                        currentIndex: keys.indexOf(Prefs.wallpaper)
                                        onActivated: Prefs.wallpaper = keys[currentIndex]
                                    }
                                }
                                Divider {}
                                SettingRow {
                                    label: qsTr("Dock icon size")
                                    Slider {
                                        from: 32
                                        to: 56
                                        stepSize: 2
                                        Layout.preferredWidth: 190
                                        value: Prefs.dockIconSize
                                        onMoved: Prefs.dockIconSize = Math.round(value)
                                    }
                                }
                            }
                            Group {
                                SettingRow {
                                    label: qsTr("Panel opacity")
                                    Label {
                                        Layout.preferredWidth: 60
                                        text: Math.round(Prefs.opacity * 100) + "%"
                                    }
                                }
                                Slider {
                                    Layout.fillWidth: true
                                    from: .45
                                    to: 1
                                    value: Prefs.opacity
                                    onMoved: Prefs.opacity = value
                                }
                            }
                            Group {
                                Label {
                                    text: qsTr("Pinned applications")
                                    font.bold: true
                                }
                                Note {
                                    text: qsTr("Right-click an app in the launcher to pin or unpin it. Press and hold a pinned Dock icon to remove it.")
                                }
                            }
                        }
                        ColumnLayout {
                            visible: root.section === "Displays"
                            Layout.fillWidth: true
                            spacing: 18
                            Group {
                                SettingRow {
                                    label: qsTr("Brightness")
                                    Slider {
                                        Layout.preferredWidth: 200
                                        from: 5
                                        to: 100
                                        enabled: !!System.state.brightnessAvailable
                                        value: System.state.brightnessPercent || 0
                                        onMoved: {
                                            brightnessTimer.requestedValue = value;
                                            brightnessTimer.restart();
                                        }
                                        Timer {
                                            id: brightnessTimer
                                            property real requestedValue: 0
                                            interval: 180
                                            onTriggered: System.action("brightness", Math.round(requestedValue))
                                        }
                                    }
                                }
                            }
                            Loader {
                                active: root.section === "Displays"
                                Layout.fillWidth: true
                                Layout.preferredHeight: item ? item.implicitHeight : 0
                                source: active ? "DisplaysPage.qml" : ""
                            }
                        }
                        ColumnLayout {
                            id: keyboardPage
                            visible: root.section === "Keyboard"
                            Layout.fillWidth: true
                            spacing: 18
                            property var selectedLayouts: []
                            property string selectedShortcut: ""
                            property int selectedSource: 0
                            function sourceName(code) {
                                if (code === "ara")
                                    return qsTr("Arabic");
                                if (code === "us")
                                    return qsTr("English (US)");
                                return ((Keyboard.state.catalog || []).find(x => x.id === code) || ({
                                            name: code
                                        })).name;
                            }
                            function editSources() {
                                selectedLayouts = (Keyboard.state.layouts || []).slice();
                                selectedShortcut = Keyboard.state.shortcut || "";
                                selectedSource = 0;
                                sourceEditor.open();
                            }
                            Group {
                                Label {
                                    text: qsTr("Text Input")
                                    font.bold: true
                                    font.pixelSize: 16
                                }
                                SettingRow {
                                    label: qsTr("Input Sources")
                                    hint: (Keyboard.state.layouts || []).map(code => keyboardPage.sourceName(code)).join(" · ")
                                    HarborButton {
                                        objectName: "edit-input-sources"
                                        text: qsTr("Edit…")
                                        enabled: !Keyboard.busy
                                        onClicked: keyboardPage.editSources()
                                    }
                                }
                            }
                            Note {
                                text: qsTr("Use the input menu in the menu bar to change your typing language.")
                            }
                            HarborField {
                                Layout.fillWidth: true
                                placeholderText: qsTr("Type here to test your keyboard…")
                            }
                            Note {
                                text: Keyboard.message
                                visible: text.length > 0
                            }
                            Popup {
                                id: sourceEditor
                                parent: root
                                anchors.centerIn: parent
                                width: Math.min(760, root.width - 40)
                                height: Math.min(550, root.height - 40)
                                padding: root.height < 620 ? 16 : 22
                                modal: true
                                focus: true
                                closePolicy: Popup.CloseOnEscape
                                background: Rectangle {
                                    objectName: "input-sources-background"
                                    radius: 14
                                    color: root.card
                                    border.color: root.line
                                }
                                contentItem: ColumnLayout {
                                    LayoutMirroring.enabled: Prefs.language === "ar"
                                    LayoutMirroring.childrenInherit: true
                                    spacing: root.height < 620 ? 10 : 18
                                    Label {
                                        text: qsTr("Input Sources")
                                        font.pixelSize: 21
                                        font.bold: true
                                    }
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        spacing: 22
                                        Rectangle {
                                            Layout.preferredWidth: 230
                                            Layout.fillHeight: true
                                            radius: 8
                                            color: Prefs.dark ? "#252529" : "#f5f5f7"
                                            border.color: root.line
                                            ColumnLayout {
                                                anchors.fill: parent
                                                anchors.margins: 8
                                                spacing: 5
                                                ListView {
                                                    id: configuredSources
                                                    Layout.fillWidth: true
                                                    Layout.fillHeight: true
                                                    clip: true
                                                    spacing: 3
                                                    model: keyboardPage.selectedLayouts
                                                    delegate: Button {
                                                        required property string modelData
                                                        required property int index
                                                        objectName: "configured-input-" + modelData
                                                        width: ListView.view.width
                                                        height: 52
                                                        onClicked: keyboardPage.selectedSource = index
                                                        background: Rectangle {
                                                            radius: 6
                                                            color: keyboardPage.selectedSource === index ? Prefs.accent : parent.hovered ? root.line : "transparent"
                                                        }
                                                        contentItem: RowLayout {
                                                            spacing: 10
                                                            Text {
                                                                text: modelData === "ara" ? "ع" : modelData.toUpperCase()
                                                                font.pixelSize: 18
                                                                color: keyboardPage.selectedSource === index ? "white" : root.ink
                                                                Layout.preferredWidth: 30
                                                                horizontalAlignment: Text.AlignHCenter
                                                            }
                                                            Text {
                                                                text: keyboardPage.sourceName(modelData)
                                                                color: keyboardPage.selectedSource === index ? "white" : root.ink
                                                                font.pixelSize: 13
                                                                Layout.fillWidth: true
                                                                elide: Text.ElideRight
                                                            }
                                                        }
                                                    }
                                                }
                                                Divider {}
                                                RowLayout {
                                                    spacing: 4
                                                    HarborButton {
                                                        objectName: "add-input"
                                                        text: "+"
                                                        Accessible.name: qsTr("Add input source")
                                                        enabled: keyboardPage.selectedLayouts.length < 4
                                                        onClicked: {
                                                            candidateSearch.text = "";
                                                            sourceChooser.candidate = "";
                                                            sourceChooser.open();
                                                        }
                                                    }
                                                    HarborButton {
                                                        objectName: "remove-input"
                                                        text: "−"
                                                        Accessible.name: qsTr("Remove input source")
                                                        enabled: keyboardPage.selectedLayouts.length > 1
                                                        onClicked: {
                                                            keyboardPage.selectedLayouts = keyboardPage.selectedLayouts.filter((x, i) => i !== keyboardPage.selectedSource);
                                                            keyboardPage.selectedSource = Math.max(0, Math.min(keyboardPage.selectedSource, keyboardPage.selectedLayouts.length - 1));
                                                        }
                                                    }
                                                    Item {
                                                        Layout.fillWidth: true
                                                    }
                                                }
                                            }
                                        }
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            spacing: root.height < 620 ? 8 : 16
                                            Rectangle {
                                                Layout.alignment: Qt.AlignHCenter
                                                width: root.height < 620 ? 56 : 76
                                                height: width
                                                radius: 14
                                                color: Prefs.dark ? "#45454b" : "#f0f0f5"
                                                border.color: root.line
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: (keyboardPage.selectedLayouts[keyboardPage.selectedSource] || "") === "ara" ? "ع" : (keyboardPage.selectedLayouts[keyboardPage.selectedSource] || "").toUpperCase()
                                                    font.pixelSize: 32
                                                    color: root.ink
                                                }
                                            }
                                            Label {
                                                text: keyboardPage.sourceName(keyboardPage.selectedLayouts[keyboardPage.selectedSource] || "")
                                                font.pixelSize: 18
                                                font.bold: true
                                                horizontalAlignment: Text.AlignHCenter
                                            }
                                            Note {
                                                text: keyboardPage.selectedSource === 0 ? qsTr("Default input source") : qsTr("Available from the input menu")
                                                horizontalAlignment: Text.AlignHCenter
                                            }
                                            HarborButton {
                                                Layout.alignment: Qt.AlignHCenter
                                                text: qsTr("Make Default")
                                                enabled: keyboardPage.selectedSource > 0
                                                onClicked: {
                                                    let v = keyboardPage.selectedLayouts.slice();
                                                    let selected = v.splice(keyboardPage.selectedSource, 1)[0];
                                                    v.unshift(selected);
                                                    keyboardPage.selectedLayouts = v;
                                                    keyboardPage.selectedSource = 0;
                                                }
                                            }
                                            Item {
                                                Layout.fillHeight: true
                                            }
                                            Note {
                                                text: qsTr("Add up to four input sources. The first source is your default.")
                                            }
                                        }
                                    }
                                    Divider {}
                                    SettingRow {
                                        label: qsTr("Switch input source")
                                        Select {
                                            objectName: "input-shortcut"
                                            Layout.preferredWidth: 190
                                            model: [qsTr("None"), "Alt + Shift", "Ctrl + Shift", "Super + Space", "Ctrl + Space"]
                                            property var values: ["", "grp:alt_shift_toggle", "grp:ctrl_shift_toggle", "grp:win_space_toggle", "grp:ctrl_space_toggle"]
                                            currentIndex: Math.max(0, values.indexOf(keyboardPage.selectedShortcut))
                                            onActivated: keyboardPage.selectedShortcut = values[currentIndex]
                                        }
                                    }
                                    Note {
                                        text: qsTr("Super is the Windows or Command key. Existing layout variants are preserved.")
                                    }
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Item {
                                            Layout.fillWidth: true
                                        }
                                        HarborButton {
                                            objectName: "cancel-input-sources"
                                            text: qsTr("Cancel")
                                            onClicked: sourceEditor.close()
                                        }
                                        HarborButton {
                                            objectName: "done-input-sources"
                                            text: qsTr("Done")
                                            prominent: true
                                            enabled: !Keyboard.busy && keyboardPage.selectedLayouts.length > 0
                                            onClicked: {
                                                Keyboard.apply(keyboardPage.selectedLayouts, keyboardPage.selectedShortcut);
                                                sourceEditor.close();
                                            }
                                        }
                                    }
                                }
                            }
                            Popup {
                                id: sourceChooser
                                parent: root
                                anchors.centerIn: parent
                                width: Math.min(480, root.width - 60)
                                height: Math.min(450, root.height - 60)
                                padding: 22
                                modal: true
                                focus: true
                                closePolicy: Popup.CloseOnEscape
                                property string candidate: ""
                                property var matches: (Keyboard.state.catalog || []).filter(x => (x.name + " " + x.id + " " + keyboardPage.sourceName(x.id)).toLowerCase().includes(candidateSearch.text.toLowerCase()))
                                background: Rectangle {
                                    radius: 14
                                    color: root.card
                                    border.color: root.line
                                }
                                contentItem: ColumnLayout {
                                    LayoutMirroring.enabled: Prefs.language === "ar"
                                    LayoutMirroring.childrenInherit: true
                                    spacing: 14
                                    Label {
                                        text: qsTr("Add Input Source")
                                        font.bold: true
                                        font.pixelSize: 19
                                    }
                                    HarborField {
                                        id: candidateSearch
                                        objectName: "input-source-search"
                                        Layout.fillWidth: true
                                        placeholderText: qsTr("Search languages")
                                    }
                                    ListView {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        clip: true
                                        spacing: 3
                                        model: sourceChooser.matches
                                        delegate: Button {
                                            required property var modelData
                                            objectName: "input-candidate-" + modelData.id
                                            width: ListView.view.width
                                            height: 42
                                            enabled: !keyboardPage.selectedLayouts.includes(modelData.id)
                                            onClicked: sourceChooser.candidate = modelData.id
                                            background: Rectangle {
                                                radius: 6
                                                color: sourceChooser.candidate === parent.modelData.id ? Prefs.accent : parent.hovered ? root.line : "transparent"
                                            }
                                            contentItem: Text {
                                                text: keyboardPage.sourceName(parent.modelData.id) + (keyboardPage.selectedLayouts.includes(parent.modelData.id) ? " ✓" : "")
                                                color: sourceChooser.candidate === parent.modelData.id ? "white" : parent.enabled ? root.ink : root.muted
                                                font.pixelSize: 14
                                                verticalAlignment: Text.AlignVCenter
                                                elide: Text.ElideRight
                                            }
                                        }
                                    }
                                    Note {
                                        visible: sourceChooser.matches.length === 0
                                        text: qsTr("No input sources found")
                                    }
                                    RowLayout {
                                        Item {
                                            Layout.fillWidth: true
                                        }
                                        HarborButton {
                                            text: qsTr("Cancel")
                                            onClicked: sourceChooser.close()
                                        }
                                        HarborButton {
                                            objectName: "confirm-add-input"
                                            text: qsTr("Add")
                                            prominent: true
                                            enabled: sourceChooser.candidate.length > 0 && keyboardPage.selectedLayouts.length < 4 && !keyboardPage.selectedLayouts.includes(sourceChooser.candidate)
                                            onClicked: {
                                                keyboardPage.selectedLayouts = keyboardPage.selectedLayouts.concat([sourceChooser.candidate]);
                                                keyboardPage.selectedSource = keyboardPage.selectedLayouts.length - 1;
                                                sourceChooser.close();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        ColumnLayout {
                            visible: root.section === "Battery"
                            Layout.fillWidth: true
                            spacing: 18
                            Loader {
                                active: root.section === "Battery"
                                Layout.fillWidth: true
                                Layout.preferredHeight: item ? item.implicitHeight : 0
                                source: active ? "PowerPage.qml" : ""
                            }
                            Group {
                                visible: !!System.state.batteryAvailable
                                SettingRow {
                                    label: qsTr("Battery", "battery status")
                                    hint: System.state.batteryCharging ? qsTr("Charging") : qsTr("On battery / fully charged")
                                    Label {
                                        text: Math.round(System.state.batteryPercent || 0) + "%"
                                        Layout.preferredWidth: 70
                                    }
                                }
                            }
                            Group {
                                Label {
                                    text: qsTr("Energy mode")
                                    font.bold: true
                                }
                                Note {
                                    text: qsTr("Current mode: ") + (System.state.power || qsTr("Unavailable"))
                                }
                                Repeater {
                                    model: [
                                        {
                                            id: "power-saver",
                                            en: QT_TR_NOOP("Low Power")
                                        },
                                        {
                                            id: "balanced",
                                            en: QT_TR_NOOP("Balanced")
                                        },
                                        {
                                            id: "performance",
                                            en: QT_TR_NOOP("Performance")
                                        }
                                    ].filter(x => (System.state.powerProfiles || []).includes(x.id))
                                    delegate: RadioButton {
                                        required property var modelData
                                        text: qsTr(modelData.en)
                                        checked: System.state.power === modelData.id
                                        enabled: !!System.state.powerAvailable && !System.busy
                                        onClicked: System.action("power", modelData.id)
                                    }
                                }
                            }
                            Note {
                                text: qsTr("Available modes depend on your hardware and power service.")
                            }
                        }
                        ColumnLayout {
                            visible: root.section === "About"
                            Layout.fillWidth: true
                            spacing: 18
                            Rectangle {
                                width: 80
                                height: 80
                                radius: 18
                                Layout.alignment: Qt.AlignHCenter
                                color: "#4888db"
                                Text {
                                    anchors.centerIn: parent
                                    text: "◈"
                                    color: "white"
                                    font.pixelSize: 60
                                }
                            }
                            Label {
                                text: "Harbor Desktop"
                                font.pixelSize: 26
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                            }
                            Note {
                                text: qsTr("Version ") + HarborVersion
                                horizontalAlignment: Text.AlignHCenter
                            }
                            Group {
                                SettingRow {
                                    label: qsTr("Window system")
                                    Label {
                                        text: "KWin · Wayland"
                                        Layout.preferredWidth: 170
                                    }
                                }
                                Divider {}
                                SettingRow {
                                    label: qsTr("Interface")
                                    Label {
                                        text: "Harbor · Qt 6"
                                        Layout.preferredWidth: 170
                                    }
                                }
                                Divider {}
                                SettingRow {
                                    label: qsTr("License")
                                    Label {
                                        text: "GPL-3.0-or-later"
                                        Layout.preferredWidth: 170
                                    }
                                }
                            }
                            Group {
                                SettingRow {
                                    label: qsTr("Operating system")
                                    Label {
                                        text: System.state.osName || "Debian Linux"
                                        Layout.preferredWidth: 220
                                    }
                                }
                                Divider {}
                                SettingRow {
                                    label: qsTr("Architecture")
                                    Label {
                                        text: System.state.architecture || "—"
                                        Layout.preferredWidth: 220
                                    }
                                }
                            }
                            Note {
                                text: qsTr("An independent open-source desktop with original artwork. Plasma Shell is not used.")
                            }
                        }
                        Note {
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
