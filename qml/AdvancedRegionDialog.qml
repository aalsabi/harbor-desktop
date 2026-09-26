import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RegionDialog {
    id: advanced
    required property Item page
    property alias tabIndex: tabs.currentIndex
    component SegmentTab: TabButton {
        id: segment
        implicitHeight: 30
        contentItem: Text {
            text: segment.text
            font.pixelSize: 13
            color: segment.checked ? "white" : RegionTheme.ink
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            radius: 6
            color: segment.checked ? Prefs.accent : "transparent"
        }
    }
    objectName: "advancedRegionDialog"
    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(680, parent ? parent.width - 32 : 680)
    height: Math.min(690, parent ? parent.height - 32 : 690)
    modal: true
    closePolicy: Popup.CloseOnEscape
    title: qsTr("Advanced Language & Region")
    contentItem: ColumnLayout {
        LayoutMirroring.enabled: RegionTheme.arabic
        LayoutMirroring.childrenInherit: true
        spacing: 14
        RegionNote {
            visible: !!page.advancedSample.error
            text: page.advancedSample.error || ""
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
                border.color: RegionTheme.line
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
            onCurrentIndexChanged: page.activePatternField = null
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
                    RegionHeading {
                        text: qsTr("Format language")
                    }
                    RegionChoice {
                        choices: Region.languages
                        selected: page.advancedDraft.formatLanguage || "en"
                        Accessible.name: qsTr("Format language")
                        onChosen: page.stageAdvanced("formatLanguage", value)
                    }
                    RegionHeading {
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
                            text: page.advancedDraft.numberGroup || ""
                            maximumLength: 4
                            Accessible.name: qsTr("Number grouping separator")
                            onTextEdited: page.stageAdvanced("numberGroup", text)
                        }
                        HarborLabel {
                            text: qsTr("Decimal")
                        }
                        HarborField {
                            objectName: "numberDecimal"
                            Layout.fillWidth: true
                            text: page.advancedDraft.numberDecimal || ""
                            maximumLength: 4
                            Accessible.name: qsTr("Number decimal separator")
                            onTextEdited: page.stageAdvanced("numberDecimal", text)
                        }
                    }
                    RegionNote {
                        text: page.advancedSample.number || ""
                    }
                    RegionHeading {
                        text: qsTr("Currency")
                    }
                    GridLayout {
                        columns: 2
                        Layout.fillWidth: true
                        columnSpacing: 12
                        HarborLabel {
                            text: qsTr("Currency")
                        }
                        RegionChoice {
                            objectName: "currencyCode"
                            choices: Region.currencies
                            selected: page.advancedDraft.currency || ""
                            Accessible.name: qsTr("Currency")
                            onChosen: page.stageAdvanced("currency", value)
                        }
                        HarborLabel {
                            text: qsTr("Grouping")
                        }
                        HarborField {
                            Layout.fillWidth: true
                            text: page.advancedDraft.currencyGroup || ""
                            maximumLength: 4
                            Accessible.name: qsTr("Currency grouping separator")
                            onTextEdited: page.stageAdvanced("currencyGroup", text)
                        }
                        HarborLabel {
                            text: qsTr("Decimal")
                        }
                        HarborField {
                            Layout.fillWidth: true
                            text: page.advancedDraft.currencyDecimal || ""
                            maximumLength: 4
                            Accessible.name: qsTr("Currency decimal separator")
                            onTextEdited: page.stageAdvanced("currencyDecimal", text)
                        }
                    }
                    RegionNote {
                        text: page.advancedSample.currency || ""
                    }
                    RegionHeading {
                        text: qsTr("Measurement units")
                    }
                    RegionChoice {
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
                        selected: page.advancedDraft.measurement || "metric"
                        Accessible.name: qsTr("Measurement units")
                        onChosen: page.stageAdvanced("measurement", value)
                    }
                    RegionNote {
                        text: page.advancedSample.measurementExample || ""
                    }
                }
                ColumnLayout {
                    visible: tabs.currentIndex > 0
                    Layout.fillWidth: true
                    spacing: 10
                    RegionNote {
                        text: qsTr("Select a format field, then insert a component below or type a custom pattern. Previews update as you edit.")
                    }
                    Repeater {
                        model: 4
                        delegate: ColumnLayout {
                            id: formatRow
                            required property int index
                            Layout.fillWidth: true
                            spacing: 4
                            RegionHeading {
                                text: [qsTr("Short"), qsTr("Medium"), qsTr("Long"), qsTr("Full")][index]
                            }
                            HarborField {
                                property int patternIndex: formatRow.index
                                objectName: "formatPattern" + patternIndex
                                Layout.fillWidth: true
                                LayoutMirroring.enabled: false
                                text: ((tabs.currentIndex === 1 ? page.advancedDraft.dateFormats : page.advancedDraft.timeFormats) || [])[patternIndex] || ""
                                Accessible.name: (tabs.currentIndex === 1 ? qsTr("Date format ") : qsTr("Time format ")) + (patternIndex + 1)
                                onActiveFocusChanged: if (activeFocus)
                                    page.activePatternField = this
                                onTextEdited: page.setPattern(patternIndex, text)
                            }
                            RegionNote {
                                text: ((tabs.currentIndex === 1 ? page.advancedSample.dates : page.advancedSample.times) || [])[index] || ""
                            }
                        }
                    }
                    RegionHeading {
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
                                enabled: page.activePatternField !== null
                                Accessible.name: qsTr("Insert ") + modelData.name
                                onClicked: page.insertToken(modelData.token)
                            }
                        }
                    }
                    RegionNote {
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
                            text: page.advancedDraft.am || ""
                            Accessible.name: qsTr("AM label")
                            onTextEdited: page.stageAdvanced("am", text)
                        }
                        HarborLabel {
                            text: qsTr("After noon")
                        }
                        HarborField {
                            Layout.fillWidth: true
                            text: page.advancedDraft.pm || ""
                            Accessible.name: qsTr("PM label")
                            onTextEdited: page.stageAdvanced("pm", text)
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
                onClicked: page.restoreAdvanced()
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
                enabled: !page.advancedSample.error
                onClicked: {
                    page.draft = page.copy(page.advancedDraft);
                    page.status = "";
                    advanced.close();
                }
            }
        }
    }
}
