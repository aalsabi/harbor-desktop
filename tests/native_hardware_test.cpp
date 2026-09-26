#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include "core/AudioStreams.h"
#include "core/DisplaySettings.h"
#include "core/PowerSettings.h"
static void write(const QString &path,const QByteArray &data){QFile f(path);if(f.open(QIODevice::WriteOnly|QIODevice::Truncate))f.write(data);}
class HardwareTest:public QObject {
 Q_OBJECT
private slots:
 void parsers(){
  bool valid=false;auto streams=AudioStreams::parse(R"([{"index":17,"sink":4,"mute":false,"properties":{"application.name":"Player","media.name":"Song"},"volume":{"front-left":{"value":32768},"front-right":{"value":65536}},"volume_writable":true}])",true,&valid);QVERIFY(valid);QCOMPARE(streams.size(),1);QCOMPARE(streams[0].toMap()["volume"].toInt(),75);QCOMPARE(streams[0].toMap()["name"].toString(),QString("Player"));AudioStreams::parse("bad JSON",true,&valid);QVERIFY(!valid);
  auto monitors=DisplaySettings::parseMonitors("Display 1\n I2C bus: /dev/i2c-5\n Monitor: DEL:Office\nInvalid display\n I2C bus: /dev/i2c-9\n");QCOMPARE(monitors.size(),1);QCOMPARE(monitors[0].toMap()["bus"].toInt(),5);
  auto vcp=DisplaySettings::parseVcp("VCP 10 C 75 150\n");QCOMPARE(vcp["percent"].toInt(),50);QVERIFY(DisplaySettings::parseVcp("VCP 10 ERR").isEmpty());
 }
 void fakeCommands(){
  QTemporaryDir tmp;auto path=qgetenv("PATH");auto root=tmp.path();qputenv("PATH",root.toUtf8());qputenv("HARBOR_HARDWARE_TEST",root.toUtf8());
  const QByteArray script=R"(#!/usr/bin/python3
import json,os,pathlib,sys
r=pathlib.Path(os.environ['HARBOR_HARDWARE_TEST']);args=sys.argv[1:]
with (r/'calls').open('a') as f:f.write(json.dumps(args)+'\n')
if (r/'reject').exists():sys.exit('Test action rejected')
if args==['-f','json','list','sinks']:print('[{"index":4,"name":"output","description":"Speakers"},{"index":5,"name":"headphones"}]')
elif args==['-f','json','list','sink-inputs']:
 s=json.loads((r/'stream').read_text()) if (r/'stream').exists() else {'index':17,'sink':4,'mute':False,'volume':{'mono':{'value':32768}},'properties':{'application.name':'Player'}}
 print(json.dumps([s]))
elif args[0].startswith('set-sink-input') or args[0]=='move-sink-input':
 s={'index':17,'sink':4,'mute':False,'volume':{'mono':{'value':32768}},'properties':{'application.name':'Player'}}
 if (r/'stream').exists():s=json.loads((r/'stream').read_text())
 if args[0]=='set-sink-input-volume':s['volume']['mono']['value']=int(int(args[2][:-1])*65536/100)
 elif args[0]=='set-sink-input-mute':s['mute']=args[2]=='1'
 else:s['sink']=int(args[2])
 (r/'stream').write_text(json.dumps(s))
elif args==['-j']:print('{"outputs":[{"id":1,"connected":true,"enabled":true,"modes":[{"id":"1"}]}]}')
elif args==['detect','--brief']:print('Display 1\n I2C bus: /dev/i2c-5\n Monitor: Test')
elif 'getvcp' in args:print('VCP 10 C '+((r/'brightness').read_text() if (r/'brightness').exists() else '50')+' 100')
elif 'setvcp' in args:(r/'brightness').write_text(args[4])
else:sys.exit('Unexpected test command')
)";
  for(auto name:{"pactl","kscreen-doctor","ddcutil"}){write(root+"/"+name,script);QFile::setPermissions(root+"/"+name,QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);}
  AudioStreams audio;audio.refresh();QTRY_VERIFY(!audio.busy());QVERIFY(audio.available());QCOMPARE(audio.streams().size(),1);
  audio.setVolume(17,75);QTRY_VERIFY(!audio.busy());QCOMPARE(audio.streams()[0].toMap()["volume"].toInt(),75);
  audio.setMuted(17,true);QTRY_VERIFY(!audio.busy());QVERIFY(audio.streams()[0].toMap()["muted"].toBool());
  audio.move(17,5);QTRY_VERIFY(!audio.busy());QCOMPARE(audio.streams()[0].toMap()["sink"].toInt(),5);
  auto size=QFileInfo(root+"/calls").size();audio.setVolume(999,50);audio.move(17,999);audio.setVolume(17,101);QCOMPARE(QFileInfo(root+"/calls").size(),size);
  DisplaySettings displays;displays.refresh();QTRY_VERIFY(!displays.busy());displays.detectBrightness();QTRY_VERIFY(!displays.busy());QCOMPARE(displays.monitors().size(),1);displays.setBrightness(5,60);QTRY_VERIFY(!displays.busy());QCOMPARE(displays.monitors()[0].toMap()["percent"].toInt(),60);
  // A detached guard that dies after writing pending must not leave the UI busy forever.
  write(root+"/harbor-display-layout-guard",R"(#!/usr/bin/python3
import json,pathlib,sys
folder=pathlib.Path(sys.argv[sys.argv.index('--directory')+1]);folder.mkdir();(folder/'status.json').write_text('{"status":"pending"}')
)");QFile::setPermissions(root+"/harbor-display-layout-guard",QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);
  auto runtime=qgetenv("XDG_RUNTIME_DIR");QDir().mkpath(root+"/runtime");QFile::setPermissions(root+"/runtime",QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);qputenv("XDG_RUNTIME_DIR",(root+"/runtime").toUtf8());
  DisplaySettings stale(nullptr,1200);stale.refresh();QTRY_VERIFY(!stale.busy());stale.apply(1,0,0,1,"1",false);QTRY_VERIFY(stale.pending());QTRY_VERIFY_WITH_TIMEOUT(!stale.busy(),3000);QVERIFY(!stale.pending());QVERIFY(stale.error().contains("unknown"));
  if(runtime.isNull())qunsetenv("XDG_RUNTIME_DIR");else qputenv("XDG_RUNTIME_DIR",runtime);
  write(root+"/reject","yes");audio.setMuted(17,false);QTRY_VERIFY(!audio.busy());QVERIFY(audio.error().contains("Test action rejected"));
  qputenv("PATH",path);qunsetenv("HARBOR_HARDWARE_TEST");
 }
 void batteryCapability(){QTemporaryDir tmp;QDir().mkpath(tmp.path()+"/BAT0");write(tmp.path()+"/BAT0/type","Battery\n");write(tmp.path()+"/BAT0/charge_control_end_threshold","80\n");write(tmp.path()+"/BAT0/capacity","62\n");auto batteries=PowerSettings::readBatteries(tmp.path());QCOMPARE(batteries.size(),1);QVERIFY(batteries[0].toMap()["canLimit"].toBool());QVERIFY(!batteries[0].toMap()["canStart"].toBool());PowerSettings power(nullptr,tmp.path()+"/power.json",tmp.path());power.refresh();QCOMPARE(power.batteries().size(),1);power.setChargeLimits("../invalid",50,80);QVERIFY(!power.error().isEmpty());}
};
QTEST_GUILESS_MAIN(HardwareTest)
#include "native_hardware_test.moc"
