pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    objectName: "languageRegionPage"
    spacing: 18
    signal keyboardRequested
    property var draft: ({})
    property var advancedDraft: ({})
    property int selectedLanguage: 0
    property string status: ""
    property bool ready: false
    readonly property bool arabic: Prefs.language === "ar"
    readonly property color ink: Prefs.dark ? "#eeeeef" : "#26262a"
    readonly property color muted: Prefs.dark ? "#aaaab2" : "#696971"
    readonly property color line: Prefs.dark ? "#505057" : "#d2d2d8"
    readonly property var sample: ready ? Region.preview(draft) : ({})
    readonly property var advancedSample: advanced.visible ? Region.preview(advancedDraft) : ({})
    property var activePatternField: null
    LayoutMirroring.enabled: arabic
    LayoutMirroring.childrenInherit: true
    function copy(value) {
        return JSON.parse(JSON.stringify(value));
    }
    function stage(key, value) {
        var next = copy(draft);
        next[key] = value;
        draft = next;
        status = "";
    }
    function stageAdvanced(key, value) {
        var next = copy(advancedDraft);
        next[key] = value;
        advancedDraft = next;
    }
    function language(code) {
        return Region.languages.find(function (l) {
            return l.code === code;
        }) || ({
                code: code,
                name: code,
                nativeName: code
            });
    }
    function languageTitle(code) {
        var l = language(code);
        return l.nativeName === l.name ? l.name : l.nativeName + " — " + l.name;
    }
    function setHour24(enabled) {
        var next = copy(draft);
        next.hour24 = enabled;
        next.timeFormats = next.timeFormats.map(function (pattern) {
            var parts = pattern.split(/('(?:[^']|'')*')/g);
            var hasPeriod = false;
            for (var i = 0; i < parts.length; i += 2) {
                parts[i] = parts[i].replace(/AP|ap|H{1,2}|h{1,2}/g, function (token) {
                    if (token.toLowerCase() === "ap") {
                        hasPeriod = true;
                        return enabled ? "" : token;
                    }
                    return enabled ? "HH" : (token.length === 2 ? "hh" : "h");
                });
            }
            var result = parts.join("").trim();
            return !enabled && !hasPeriod ? result + " AP" : result;
        });
        draft = next;
        status = "";
    }
    function changeRegion(code) {
        var next = copy(Region.defaults(code));
        next.languages = draft.languages.slice();
        draft = next;
        status = "";
    }
    function moveLanguage(offset) {
        var values = draft.languages.slice();
        var destination = selectedLanguage + offset;
        if (destination < 0 || destination >= values.length)
            return;
        var value = values.splice(selectedLanguage, 1)[0];
        values.splice(destination, 0, value);
        stage("languages", values);
        selectedLanguage = destination;
    }
    function restoreMain() {
        var next = copy(Region.defaults(draft.region));
        next.languages = draft.languages.slice();
        draft = next;
        status = qsTr("Defaults restored in this draft. Choose Apply to save.");
    }
    function restoreAdvanced() {
        var defaults = Region.defaults(advancedDraft.region);
        var next = copy(advancedDraft);
        var keys = tabs.currentIndex === 1 ? ["dateFormats"] : tabs.currentIndex === 2 ? ["timeFormats", "am", "pm", "hour24"] : ["formatLanguage", "numberGroup", "numberDecimal", "currency", "currencyGroup", "currencyDecimal", "measurement"];
        keys.forEach(function (key) {
            next[key] = defaults[key];
        });
        advancedDraft = next;
    }
    function openAdvanced() {
        advancedDraft = copy(draft);
        tabs.currentIndex = 0;
        activePatternField = null;
        advanced.open();
    }
    function setPattern(index, value) {
        var key = tabs.currentIndex === 1 ? "dateFormats" : "timeFormats";
        var values = (advancedDraft[key] || []).slice();
        values[index] = value;
        stageAdvanced(key, values);
        if (key === "timeFormats" && index === 0)
            stageAdvanced("hour24", !/AP|ap/.test(value.replace(/'(?:[^']|'')*'/g, "")));
    }
    function insertToken(token) {
        if (!activePatternField)
            return;
        var field = activePatternField;
        field.insert(field.cursorPosition, token);
        setPattern(field.patternIndex, field.text);
        field.forceActiveFocus();
    }
    property var generationDraft: ({})
    property var requiredLocales: []
    function saved() {
        draft = copy(Region.state);
        var supported = draft.languages.find(function (code) {
            return code.split(/[-_]/)[0] === "ar" || code.split(/[-_]/)[0] === "en";
        });
        if (supported)
            Prefs.language = supported.split(/[-_]/)[0];
        status = qsTr("Preferences saved. Sign out and back in to apply them to other applications.");
    }
    function applyChanges() {
        if (Region.busy)
            return;
        var plan = Region.generationPlan(draft);
        if (plan.error) {
            status = plan.error;
            return;
        }
        if (plan.locales.length) {
            generationDraft = copy(draft);
            requiredLocales = plan.locales;
            status = "";
            generateDialog.open();
            return;
        }
        if (Region.apply(draft))
            saved();
        else
            status = Region.message || qsTr("The settings could not be applied.");
    }
    Connections {
        target: Region
        function onGenerationFinished(success) {
            if (success) {
                generateDialog.close();
                root.saved();
            } else
                root.status = Region.message || qsTr("Generation was cancelled or failed. Your settings were not changed.");
        }
    }
    Component.onCompleted: {
        draft = copy(Region.state);
        ready = true;
    }

    component Note: HarborLabel {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: root.muted
        font.pixelSize: 12
    }
    component Heading: HarborLabel {
        font.bold: true
        font.pixelSize: 13
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
    }
    component Choice: ComboBox {
        id: choice
        property var choices: []
        property string selected: ""
        signal chosen(string value)
        model: choices
        textRole: "name"
        valueRole: "code"
        Layout.fillWidth: true
        implicitHeight: 30
        currentIndex: {
            for (var i = 0; i < choices.length; i++)
                if (String(choices[i].code) === selected)
                    return i;
            return -1;
        }
        onActivated: chosen(String(currentValue))
        Accessible.name: displayText
        leftPadding: root.arabic ? 32 : 10
        rightPadding: root.arabic ? 10 : 32
        contentItem: Text {
            text: choice.displayText
            color: root.ink
            font.pixelSize: 13
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            horizontalAlignment: root.arabic ? Text.AlignRight : Text.AlignLeft
        }
        indicator: Text {
            x: root.arabic ? 10 : choice.width - width - 10
            y: (choice.height - height) / 2
            text: "⌄"
            color: root.muted
            font.pixelSize: 17
        }
        background: Rectangle {
            radius: 6
            color: Prefs.dark ? "#3c3c42" : "#ffffff"
            border.color: choice.activeFocus ? Prefs.accent : root.line
        }
        delegate: ItemDelegate {
            id: option
            required property int index
            required property var modelData
            width: choice.width
            height: 34
            highlighted: choice.highlightedIndex === index
            contentItem: Text {
                text: option.modelData.name
                color: option.highlighted ? "white" : root.ink
                font.pixelSize: 13
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                radius: 4
                color: option.highlighted ? Prefs.accent : "transparent"
            }
        }
        popup: Popup {
            y: choice.height + 4
            width: choice.width
            padding: 5
            implicitHeight: Math.min(contentItem.implicitHeight + 10, 280)
            background: Rectangle {
                radius: 7
                color: Prefs.dark ? "#323238" : "#ffffff"
                border.color: root.line
            }
            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: choice.popup.visible ? choice.delegateModel : null
                currentIndex: choice.highlightedIndex
                ScrollBar.vertical: ScrollBar {}
            }
        }
    }
    component ThemedDialog: Dialog {
        id: themedDialog
        padding: 18
        spacing: 12
        background: Rectangle {
            radius: 12
            color: Prefs.dark ? "#29292e" : "#f7f7f9"
            border.color: root.line
        }
        header: HarborLabel {
            text: themedDialog.title
            font.bold: true
            font.pixelSize: 14
            padding: 18
            bottomPadding: 6
            wrapMode: Text.WordWrap
            horizontalAlignment: root.arabic ? Text.AlignRight : Text.AlignLeft
        }
    }
    component SegmentTab: TabButton {
        id: segment
        implicitHeight: 30
        contentItem: Text {
            text: segment.text
            font.pixelSize: 13
            color: segment.checked ? "white" : root.ink
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            radius: 6
            color: segment.checked ? Prefs.accent : "transparent"
        }
    }
    component Pane: Frame {
        padding: 14
        background: Rectangle {
            color: Prefs.dark ? "#2e2e33" : "#f7f7f9"
            radius: 8
            border.color: root.line
        }
    }

    GridLayout {
        Layout.fillWidth: true
        columns: root.width >= 600 ? 2 : 1
        columnSpacing: 18
        rowSpacing: 18
        Pane {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: root.width >= 600 ? root.width * 0.47 : root.width
            contentItem: ColumnLayout {
                spacing: 9
                Heading {
                    text: qsTr("Preferred languages")
                }
                Note {
                    text: qsTr("Menu languages, not keyboard layouts. Keep only English for English-only menus.")
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 184
                    color: Prefs.dark ? "#222226" : "white"
                    border.color: root.line
                    radius: 4
                    ListView {
                        id: preferred
                        objectName: "preferredLanguages"
                        anchors.fill: parent
                        anchors.margins: 1
                        clip: true
                        model: root.draft.languages || []
                        currentIndex: root.selectedLanguage
                        ScrollBar.vertical: ScrollBar {}
                        delegate: ItemDelegate {
                            id: languageRow
                            required property string modelData
                            required property int index
                            width: ListView.view.width
                            height: 48
                            highlighted: index === root.selectedLanguage
                            onClicked: root.selectedLanguage = index
                            Accessible.name: root.languageTitle(modelData) + (index === 0 ? qsTr(", Primary") : "")
                            background: Rectangle {
                                color: languageRow.highlighted ? Prefs.accent : "transparent"
                            }
                            contentItem: Column {
                                spacing: 2
                                Text {
                                    width: parent.width
                                    text: root.languageTitle(modelData)
                                    color: languageRow.highlighted ? "white" : root.ink
                                    elide: Text.ElideRight
                                    font.pixelSize: 13
                                }
                                Text {
                                    text: index === 0 ? qsTr("Primary") : qsTr("Alternative")
                                    color: languageRow.highlighted ? "#eeffffff" : root.muted
                                    font.pixelSize: 11
                                }
                            }
                        }
                    }
                }
                RowLayout {
                    spacing: 5
                    HarborButton {
                        objectName: "addLanguage"
                        text: "+"
                        Layout.preferredWidth: 34
                        Accessible.name: qsTr("Add language")
                        onClicked: {
                            languageSearch.text = "";
                            addDialog.selectedCode = "";
                            addDialog.open();
                        }
                    }
                    HarborButton {
                        objectName: "removeLanguage"
                        text: "−"
                        Layout.preferredWidth: 34
                        enabled: (root.draft.languages || []).length > 1
                        Accessible.name: qsTr("Remove selected language")
                        onClicked: {
                            var a = root.draft.languages.slice();
                            a.splice(root.selectedLanguage, 1);
                            root.stage("languages", a);
                            root.selectedLanguage = Math.min(root.selectedLanguage, a.length - 1);
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    HarborButton {
                        text: "↑"
                        Layout.preferredWidth: 34
                        enabled: root.selectedLanguage > 0
                        Accessible.name: qsTr("Move selected language up")
                        onClicked: root.moveLanguage(-1)
                    }
                    HarborButton {
                        text: "↓"
                        Layout.preferredWidth: 34
                        enabled: root.selectedLanguage < (root.draft.languages || []).length - 1
                        Accessible.name: qsTr("Move selected language down")
                        onClicked: root.moveLanguage(1)
                    }
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 10
            Heading {
                text: qsTr("Region")
            }
            Choice {
                objectName: "regionChoice"
                choices: Region.regions
                selected: root.draft.region || ""
                Accessible.name: qsTr("Region")
                onChosen: root.changeRegion(value)
            }
            Heading {
                text: qsTr("First day of the week")
            }
            Choice {
                choices: [
                    {
                        code: "1",
                        name: qsTr("Monday")
                    },
                    {
                        code: "2",
                        name: qsTr("Tuesday")
                    },
                    {
                        code: "3",
                        name: qsTr("Wednesday")
                    },
                    {
                        code: "4",
                        name: qsTr("Thursday")
                    },
                    {
                        code: "5",
                        name: qsTr("Friday")
                    },
                    {
                        code: "6",
                        name: qsTr("Saturday")
                    },
                    {
                        code: "7",
                        name: qsTr("Sunday")
                    }
                ]
                selected: String(root.draft.firstDay || 1)
                Accessible.name: qsTr("First day of the week")
                onChosen: root.stage("firstDay", Number(value))
            }
            Heading {
                text: qsTr("Calendar")
            }
            Choice {
                choices: Region.calendars
                selected: root.draft.calendar || "Gregorian"
                Accessible.name: qsTr("Calendar")
                onChosen: root.stage("calendar", value)
            }
            CheckBox {
                objectName: "hour24"
                palette.windowText: root.ink
                palette.text: root.ink
                palette.buttonText: root.ink
                text: qsTr("Use 24-hour time")
                checked: root.draft.hour24 === true
                onClicked: root.setHour24(checked)
            }
        }
    }
    Pane {
        Layout.fillWidth: true
        contentItem: ColumnLayout {
            spacing: 7
            Heading {
                text: qsTr("Region preview")
            }
            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 16
                rowSpacing: 6
                HarborLabel {
                    text: qsTr("Date")
                    color: root.muted
                }
                HarborLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                    text: root.sample.date || ""
                }
                HarborLabel {
                    text: qsTr("Time")
                    color: root.muted
                }
                HarborLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                    text: root.sample.time || ""
                }
                HarborLabel {
                    text: qsTr("Number")
                    color: root.muted
                }
                HarborLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                    text: root.sample.number || ""
                }
                HarborLabel {
                    text: qsTr("Week")
                    color: root.muted
                }
                HarborLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: (root.sample.weekdays || []).join(" · ")
                }
                HarborLabel {
                    text: qsTr("Units")
                    color: root.muted
                }
                HarborLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: root.sample.measurementExample || ""
                }
                HarborLabel {
                    text: qsTr("Currency")
                    color: root.muted
                }
                HarborLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                    text: root.sample.currency || ""
                }
            }
        }
    }
    Note {
        visible: !!root.sample.error
        text: root.sample.error || ""
    }
    Note {
        visible: Region.localeNotice.length > 0
        text: Region.localeNotice
    }
    Note {
        text: qsTr("Harbor is translated into English and Arabic. Preferred languages and region apply to other applications after your next login. Custom date, time and number formats apply within Harbor.")
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 8
        HarborButton {
            text: qsTr("Keyboard Preferences…")
            onClicked: root.keyboardRequested()
        }
        Item {
            Layout.fillWidth: true
        }
        HarborButton {
            objectName: "advancedRegion"
            text: qsTr("Advanced…")
            onClicked: root.openAdvanced()
        }
    }
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: 1
        color: root.line
    }
    RowLayout {
        Layout.fillWidth: true
        HarborButton {
            objectName: "restoreRegion"
            text: qsTr("Restore Defaults")
            onClicked: root.restoreMain()
        }
        Item {
            Layout.fillWidth: true
        }
        HarborButton {
            objectName: "revertRegion"
            text: qsTr("Revert")
            onClicked: {
                root.draft = root.copy(Region.state);
                root.selectedLanguage = 0;
                root.status = "";
            }
        }
        HarborButton {
            objectName: "applyRegion"
            text: qsTr("Apply")
            prominent: true
            enabled: !root.sample.error && !Region.busy
            onClicked: root.applyChanges()
        }
    }
    Note {
        objectName: "regionStatus"
        visible: text.length > 0
        text: root.status
        Accessible.role: Accessible.StaticText
    }

    ThemedDialog {
        id: generateDialog
        objectName: "generateLocalesDialog"
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(500, parent ? parent.width - 32 : 500)
        modal: true
        closePolicy: Region.busy ? Popup.NoAutoClose : Popup.CloseOnEscape
        title: qsTr("Generate regional settings?")
        contentItem: ColumnLayout {
            spacing: 12
            LayoutMirroring.enabled: root.arabic
            LayoutMirroring.childrenInherit: true
            Note {
                text: qsTr("The required locales are not generated on this computer. Choose OK to generate them and apply your preferences. Administrator authentication is required.")
            }
            HarborLabel {
                Layout.fillWidth: true
                wrapMode: Text.WrapAnywhere
                text: root.requiredLocales.join(" · ")
                LayoutMirroring.enabled: false
            }
            RowLayout {
                visible: Region.busy
                BusyIndicator {
                    running: Region.busy
                    implicitWidth: 28
                    implicitHeight: 28
                }
                Note {
                    text: qsTr("Waiting for authentication or generating locales…")
                }
            }
            Note {
                objectName: "generationError"
                visible: root.status.length > 0
                text: root.status
            }
        }
        footer: Item {
            implicitHeight: 60
            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 8
                Item {
                    Layout.fillWidth: true
                }
                HarborButton {
                    objectName: "cancelGeneration"
                    text: qsTr("Cancel")
                    enabled: !Region.busy
                    onClicked: generateDialog.close()
                }
                HarborButton {
                    objectName: "confirmGeneration"
                    text: qsTr("OK")
                    prominent: true
                    enabled: !Region.busy
                    onClicked: {
                        root.status = "";
                        Region.generateAndApply(root.generationDraft);
                    }
                }
            }
        }
    }

    ThemedDialog {
        id: addDialog
        objectName: "addLanguageDialog"
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(480, parent ? parent.width - 32 : 480)
        height: Math.min(540, parent ? parent.height - 32 : 540)
        modal: true
        closePolicy: Popup.CloseOnEscape
        title: qsTr("Add a preferred language")
        property string selectedCode: ""
        contentItem: ColumnLayout {
            LayoutMirroring.enabled: root.arabic
            LayoutMirroring.childrenInherit: true
            spacing: 10
            HarborField {
                id: languageSearch
                objectName: "languageSearch"
                Layout.fillWidth: true
                placeholderText: qsTr("Search languages")
                Accessible.name: placeholderText
            }
            ListView {
                id: availableLanguages
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: Region.languages.filter(function (l) {
                    var q = languageSearch.text.toLowerCase();
                    return (root.draft.languages || []).indexOf(l.code) < 0 && (l.name + " " + l.nativeName + " " + l.code).toLowerCase().indexOf(q) >= 0;
                })
                ScrollBar.vertical: ScrollBar {}
                delegate: ItemDelegate {
                    id: availableLanguage
                    required property var modelData
                    width: ListView.view.width
                    height: 44
                    text: root.languageTitle(modelData.code)
                    highlighted: addDialog.selectedCode === modelData.code
                    contentItem: Text {
                        text: availableLanguage.text
                        color: availableLanguage.highlighted ? "white" : root.ink
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        font.pixelSize: 13
                    }
                    background: Rectangle {
                        radius: 5
                        color: availableLanguage.highlighted ? Prefs.accent : availableLanguage.hovered ? (Prefs.dark ? "#414149" : "#e7e7ed") : "transparent"
                    }
                    onClicked: addDialog.selectedCode = modelData.code
                }
                HarborLabel {
                    anchors.centerIn: parent
                    visible: availableLanguages.count === 0
                    text: qsTr("No matching languages")
                }
            }
        }
        footer: Frame {
            padding: 16
            background: Item {}
            contentItem: RowLayout {
                spacing: 8
                Item {
                    Layout.fillWidth: true
                }
                HarborButton {
                    text: qsTr("Cancel")
                    onClicked: addDialog.close()
                }
                HarborButton {
                    objectName: "confirmAddLanguage"
                    text: qsTr("Add")
                    prominent: true
                    enabled: addDialog.selectedCode.length > 0
                    onClicked: {
                        var a = root.draft.languages.slice();
                        a.push(addDialog.selectedCode);
                        root.stage("languages", a);
                        root.selectedLanguage = a.length - 1;
                        addDialog.close();
                    }
                }
            }
        }
    }
    ThemedDialog {
        id: advanced
        objectName: "advancedRegionDialog"
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(680, parent ? parent.width - 32 : 680)
        height: Math.min(690, parent ? parent.height - 32 : 690)
        modal: true
        closePolicy: Popup.CloseOnEscape
        title: qsTr("Advanced Language & Region")
        contentItem: ColumnLayout {
            LayoutMirroring.enabled: root.arabic
            LayoutMirroring.childrenInherit: true
            spacing: 14
            Note {
                visible: !!root.advancedSample.error
                text: root.advancedSample.error || ""
            }
            TabBar {
                id: tabs
                objectName: "advancedRegionTabs"
                Layout.fillWidth: true
                implicitHeight: 36
                padding: 3
                spacing: 2
                background: Rectangle {
                    radius: 9
                    color: Prefs.dark ? "#3a3a40" : "#e7e7ec"
                    border.color: root.line
                }
                SegmentTab {
                    text: qsTr("General")
                }
                SegmentTab {
                    text: qsTr("Dates")
                }
                SegmentTab {
                    text: qsTr("Times")
                }
                onCurrentIndexChanged: root.activePatternField = null
            }
            ScrollView {
                id: advancedScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: availableWidth
                clip: true
                ColumnLayout {
                    width: advancedScroll.availableWidth
                    spacing: 14
                    ColumnLayout {
                        visible: tabs.currentIndex === 0
                        Layout.fillWidth: true
                        spacing: 10
                        Heading {
                            text: qsTr("Format language")
                        }
                        Choice {
                            choices: Region.languages
                            selected: root.advancedDraft.formatLanguage || "en"
                            Accessible.name: qsTr("Format language")
                            onChosen: root.stageAdvanced("formatLanguage", value)
                        }
                        Heading {
                            text: qsTr("Number separators")
                        }
                        GridLayout {
                            columns: 2
                            Layout.fillWidth: true
                            columnSpacing: 12
                            HarborLabel {
                                text: qsTr("Grouping")
                            }
                            HarborField {
                                objectName: "numberGroup"
                                Layout.fillWidth: true
                                text: root.advancedDraft.numberGroup || ""
                                maximumLength: 4
                                Accessible.name: qsTr("Number grouping separator")
                                onTextEdited: root.stageAdvanced("numberGroup", text)
                            }
                            HarborLabel {
                                text: qsTr("Decimal")
                            }
                            HarborField {
                                objectName: "numberDecimal"
                                Layout.fillWidth: true
                                text: root.advancedDraft.numberDecimal || ""
                                maximumLength: 4
                                Accessible.name: qsTr("Number decimal separator")
                                onTextEdited: root.stageAdvanced("numberDecimal", text)
                            }
                        }
                        Note {
                            text: root.advancedSample.number || ""
                        }
                        Heading {
                            text: qsTr("Currency")
                        }
                        GridLayout {
                            columns: 2
                            Layout.fillWidth: true
                            columnSpacing: 12
                            HarborLabel {
                                text: qsTr("Currency")
                            }
                            Choice {
                                objectName: "currencyCode"
                                choices: Region.currencies
                                selected: root.advancedDraft.currency || ""
                                Accessible.name: qsTr("Currency")
                                onChosen: root.stageAdvanced("currency", value)
                            }
                            HarborLabel {
                                text: qsTr("Grouping")
                            }
                            HarborField {
                                Layout.fillWidth: true
                                text: root.advancedDraft.currencyGroup || ""
                                maximumLength: 4
                                Accessible.name: qsTr("Currency grouping separator")
                                onTextEdited: root.stageAdvanced("currencyGroup", text)
                            }
                            HarborLabel {
                                text: qsTr("Decimal")
                            }
                            HarborField {
                                Layout.fillWidth: true
                                text: root.advancedDraft.currencyDecimal || ""
                                maximumLength: 4
                                Accessible.name: qsTr("Currency decimal separator")
                                onTextEdited: root.stageAdvanced("currencyDecimal", text)
                            }
                        }
                        Note {
                            text: root.advancedSample.currency || ""
                        }
                        Heading {
                            text: qsTr("Measurement units")
                        }
                        Choice {
                            choices: [
                                {
                                    code: "metric",
                                    name: qsTr("Metric")
                                },
                                {
                                    code: "us",
                                    name: qsTr("US")
                                },
                                {
                                    code: "uk",
                                    name: qsTr("UK")
                                }
                            ]
                            selected: root.advancedDraft.measurement || "metric"
                            Accessible.name: qsTr("Measurement units")
                            onChosen: root.stageAdvanced("measurement", value)
                        }
                        Note {
                            text: root.advancedSample.measurementExample || ""
                        }
                    }
                    ColumnLayout {
                        visible: tabs.currentIndex > 0
                        Layout.fillWidth: true
                        spacing: 10
                        Note {
                            text: qsTr("Select a format field, then insert a component below or type a custom pattern. Previews update as you edit.")
                        }
                        Repeater {
                            model: 4
                            delegate: ColumnLayout {
                                id: formatRow
                                required property int index
                                Layout.fillWidth: true
                                spacing: 4
                                Heading {
                                    text: [qsTr("Short"), qsTr("Medium"), qsTr("Long"), qsTr("Full")][index]
                                }
                                HarborField {
                                    property int patternIndex: formatRow.index
                                    objectName: "formatPattern" + patternIndex
                                    Layout.fillWidth: true
                                    LayoutMirroring.enabled: false
                                    text: ((tabs.currentIndex === 1 ? root.advancedDraft.dateFormats : root.advancedDraft.timeFormats) || [])[patternIndex] || ""
                                    Accessible.name: (tabs.currentIndex === 1 ? qsTr("Date format ") : qsTr("Time format ")) + (patternIndex + 1)
                                    onActiveFocusChanged: if (activeFocus)
                                        root.activePatternField = this
                                    onTextEdited: root.setPattern(patternIndex, text)
                                }
                                Note {
                                    text: ((tabs.currentIndex === 1 ? root.advancedSample.dates : root.advancedSample.times) || [])[index] || ""
                                }
                            }
                        }
                        Heading {
                            text: qsTr("Insert a component")
                        }
                        Flow {
                            Layout.fillWidth: true
                            spacing: 6
                            Repeater {
                                model: tabs.currentIndex === 1 ? [
                                    {
                                        token: "d",
                                        name: qsTr("Day")
                                    },
                                    {
                                        token: "dddd",
                                        name: qsTr("Weekday")
                                    },
                                    {
                                        token: "M",
                                        name: qsTr("Month number")
                                    },
                                    {
                                        token: "MMMM",
                                        name: qsTr("Month name")
                                    },
                                    {
                                        token: "yyyy",
                                        name: qsTr("Year")
                                    }
                                ] : [
                                    {
                                        token: "HH",
                                        name: qsTr("Hour 24")
                                    },
                                    {
                                        token: "hh",
                                        name: qsTr("Hour 12")
                                    },
                                    {
                                        token: "mm",
                                        name: qsTr("Minute")
                                    },
                                    {
                                        token: "ss",
                                        name: qsTr("Second")
                                    },
                                    {
                                        token: "AP",
                                        name: qsTr("AM/PM")
                                    },
                                    {
                                        token: "zzz",
                                        name: qsTr("Millisecond")
                                    },
                                    {
                                        token: "t",
                                        name: qsTr("Time zone")
                                    },
                                    {
                                        token: "tttt",
                                        name: qsTr("Zone name")
                                    }
                                ]
                                delegate: HarborButton {
                                    required property var modelData
                                    text: modelData.name + " · " + modelData.token
                                    enabled: root.activePatternField !== null
                                    Accessible.name: qsTr("Insert ") + modelData.name
                                    onClicked: root.insertToken(modelData.token)
                                }
                            }
                        }
                        Note {
                            text: qsTr("Use single quotes around literal words. For example: d MMMM yyyy or hh:mm AP. A 12-hour time format needs AP.")
                        }
                        GridLayout {
                            visible: tabs.currentIndex === 2
                            Layout.fillWidth: true
                            columns: 2
                            columnSpacing: 12
                            HarborLabel {
                                text: qsTr("Before noon")
                            }
                            HarborField {
                                Layout.fillWidth: true
                                text: root.advancedDraft.am || ""
                                Accessible.name: qsTr("AM label")
                                onTextEdited: root.stageAdvanced("am", text)
                            }
                            HarborLabel {
                                text: qsTr("After noon")
                            }
                            HarborField {
                                Layout.fillWidth: true
                                text: root.advancedDraft.pm || ""
                                Accessible.name: qsTr("PM label")
                                onTextEdited: root.stageAdvanced("pm", text)
                            }
                        }
                    }
                }
            }
        }
        footer: Frame {
            padding: 16
            background: Item {}
            contentItem: RowLayout {
                spacing: 8
                Item {
                    Layout.fillWidth: true
                }
                HarborButton {
                    objectName: "restoreAdvanced"
                    text: qsTr("Restore Defaults")
                    onClicked: root.restoreAdvanced()
                }
                HarborButton {
                    objectName: "cancelAdvanced"
                    text: qsTr("Cancel")
                    onClicked: advanced.close()
                }
                HarborButton {
                    objectName: "acceptAdvanced"
                    text: qsTr("OK")
                    prominent: true
                    enabled: !root.advancedSample.error
                    onClicked: {
                        root.draft = root.copy(root.advancedDraft);
                        root.status = "";
                        advanced.close();
                    }
                }
            }
        }
    }
}
