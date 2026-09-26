#include <QtTest>
#include "core/SystemState.h"
class SystemStateTest : public QObject {
    Q_OBJECT
private slots:
    void parsesVolumeAndRealBrightness() {
        auto a = SystemState::parse("volume", "Volume: 0.42 [MUTED]");
        QCOMPARE(a["outputVolume"].toDouble(), .42);
        QVERIFY(a["outputMuted"].toBool());
        auto b = SystemState::parse("brightness", "intel_backlight,backlight,120,37%,320");
        QCOMPARE(b["brightnessPercent"].toInt(), 37);
    }
    void preservesEscapedSsid() {
        auto a = SystemState::parse(
            "networks",
            "*:Cafe\\: East:75:WPA2:AA\\:BB\\:CC\\:DD\\:EE\\:FF\n:Guest:20:--:11\\:22\\:33\\:44\\:55\\:66");
        auto rows = a["wifiNetworks"].toList();
        QCOMPARE(rows.size(), 2);
        QCOMPARE(rows[0].toMap()["name"].toString(), QString("Cafe: East"));
        QVERIFY(rows[0].toMap()["active"].toBool());
    }
    void audioNodesExcludeOtherClasses() {
        auto a = SystemState::parse(
            "audioNodes",
            R"([{"id":12,"type":"PipeWire:Interface:Node","info":{"props":{"media.class":"Audio/Sink","node.description":"Speakers"}}},{"id":13,"type":"PipeWire:Interface:Node","info":{"props":{"media.class":"Audio/Source","node.description":"Microphone"}}},{"id":14,"info":{"props":{"media.class":"Video/Source"}}}])");
        QCOMPARE(a["audioOutputs"].toList().size(), 1);
        QCOMPARE(a["audioInputs"].toList().size(), 1);
    }
    void reportsOnlyAvailableProfiles() {
        auto a = SystemState::parse("profiles", "* balanced:\n Driver: x\n power-saver:\n Driver: x\n");
        QCOMPARE(a["powerProfiles"].toStringList(), QStringList({"balanced", "power-saver"}));
    }
};
QTEST_GUILESS_MAIN(SystemStateTest)
#include "system_state_test.moc"
