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
        var keys = advanced.tabIndex === 1 ? ["dateFormats"] : advanced.tabIndex === 2 ? ["timeFormats", "am", "pm", "hour24"] : ["formatLanguage", "numberGroup", "numberDecimal", "currency", "currencyGroup", "currencyDecimal", "measurement"];
        keys.forEach(function (key) {
            next[key] = defaults[key];
        });
        advancedDraft = next;
    }
    function openAdvanced() {
        advancedDraft = copy(draft);
        advanced.tabIndex = 0;
        activePatternField = null;
        advanced.open();
    }
    function setPattern(index, value) {
        var key = advanced.tabIndex === 1 ? "dateFormats" : "timeFormats";
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

    component Pane: Frame {
        padding: 14
        background: Rectangle {
            color: Prefs.dark ? "#2e2e33" : "#f7f7f9"
            radius: 8
            border.color: RegionTheme.line
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
                RegionHeading {
                    text: qsTr("Preferred languages")
                }
                RegionNote {
                    text: qsTr("Menu languages, not keyboard layouts. Keep only English for English-only menus.")
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 184
                    color: Prefs.dark ? "#222226" : "white"
                    border.color: RegionTheme.line
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
                                    color: languageRow.highlighted ? "white" : RegionTheme.ink
                                    elide: Text.ElideRight
                                    font.pixelSize: 13
                                }
                                Text {
                                    text: index === 0 ? qsTr("Primary") : qsTr("Alternative")
                                    color: languageRow.highlighted ? "#eeffffff" : RegionTheme.muted
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
                            addDialog.searchText = "";
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
            RegionHeading {
                text: qsTr("Region")
            }
            RegionChoice {
                objectName: "regionChoice"
                choices: Region.regions
                selected: root.draft.region || ""
                Accessible.name: qsTr("Region")
                onChosen: root.changeRegion(value)
            }
            RegionHeading {
                text: qsTr("First day of the week")
            }
            RegionChoice {
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
            RegionHeading {
                text: qsTr("Calendar")
            }
            RegionChoice {
                choices: Region.calendars
                selected: root.draft.calendar || "Gregorian"
                Accessible.name: qsTr("Calendar")
                onChosen: root.stage("calendar", value)
            }
            CheckBox {
                objectName: "hour24"
                palette.windowText: RegionTheme.ink
                palette.text: RegionTheme.ink
                palette.buttonText: RegionTheme.ink
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
            RegionHeading {
                text: qsTr("Region preview")
            }
            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 16
                rowSpacing: 6
                HarborLabel {
                    text: qsTr("Date")
                    color: RegionTheme.muted
                }
                HarborLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                    text: root.sample.date || ""
                }
                HarborLabel {
                    text: qsTr("Time")
                    color: RegionTheme.muted
                }
                HarborLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                    text: root.sample.time || ""
                }
                HarborLabel {
                    text: qsTr("Number")
                    color: RegionTheme.muted
                }
                HarborLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                    text: root.sample.number || ""
                }
                HarborLabel {
                    text: qsTr("Week")
                    color: RegionTheme.muted
                }
                HarborLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: (root.sample.weekdays || []).join(" · ")
                }
                HarborLabel {
                    text: qsTr("Units")
                    color: RegionTheme.muted
                }
                HarborLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: root.sample.measurementExample || ""
                }
                HarborLabel {
                    text: qsTr("Currency")
                    color: RegionTheme.muted
                }
                HarborLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                    text: root.sample.currency || ""
                }
            }
        }
    }
    RegionNote {
        visible: !!root.sample.error
        text: root.sample.error || ""
    }
    RegionNote {
        visible: Region.localeNotice.length > 0
        text: Region.localeNotice
    }
    RegionNote {
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
        color: RegionTheme.line
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
    RegionNote {
        objectName: "regionStatus"
        visible: text.length > 0
        text: root.status
        Accessible.role: Accessible.StaticText
    }

    GenerateLocalesDialog {
        id: generateDialog
        page: root
    }

    AddLanguageDialog {
        id: addDialog
        page: root
    }
    AdvancedRegionDialog {
        id: advanced
        page: root
    }
}
