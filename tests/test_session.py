import os, pathlib, subprocess, tempfile, unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class SessionTests(unittest.TestCase):
 def test_missing_compositor_fails_without_touching_config(self):
  script=ROOT/'scripts/harbor-session'
  self.assertTrue(script.exists(), 'session launcher is not implemented')
  with tempfile.TemporaryDirectory() as d:
   config=pathlib.Path(d)/'kwinrc'; config.write_text('keep existing settings')
   r=subprocess.run(['/usr/bin/python3',str(script),'--nested'],env={**os.environ,'PATH':d,'XDG_CONFIG_HOME':d},capture_output=True,text=True,timeout=5)
   self.assertNotEqual(r.returncode,0)
   self.assertIn('kwin_wayland',r.stderr)
   self.assertEqual(config.read_text(),'keep existing settings')
 def test_session_isolates_kwin_config(self):
  script=ROOT/'scripts/harbor-session'
  with tempfile.TemporaryDirectory() as d:
   folder=pathlib.Path(d); (folder/'kwinrc').write_text('preserve')
   fake=folder/'kwin_wayland'; fake.write_text('#!/usr/bin/python3\nimport os,pathlib\np=pathlib.Path(os.environ["XDG_CONFIG_HOME"])\nassert p.name == "session"\nassert (p/"kwinrc").exists()\n'); fake.chmod(0o755)
   sh=folder/'harbor-shell';sh.write_text('#!/bin/sh\nexit 0\n');sh.chmod(0o755)
   r=subprocess.run(['/usr/bin/python3',str(script),'--nested'],env={**os.environ,'PATH':d,'DBUS_SESSION_BUS_ADDRESS':'test','HARBOR_PRIVATE_BUS':'1','XDG_CONFIG_HOME':d},capture_output=True,text=True)
   self.assertEqual(r.returncode,0,r.stderr)
   self.assertEqual((folder/'kwinrc').read_text(),'preserve')
 def test_doctor_machine_readable(self):
  script=ROOT/'scripts/harbor-doctor'
  self.assertTrue(script.exists(), 'doctor is not implemented')
  import json
  r=subprocess.run(['/usr/bin/python3',str(script)],capture_output=True,text=True,timeout=5)
  data=json.loads(r.stdout)
  self.assertIn('kwin_wayland',data['executables'])
  self.assertFalse(data['hardwareValidated'])
if __name__=='__main__': unittest.main()
