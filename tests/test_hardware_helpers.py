import importlib.machinery,importlib.util,json,os,pathlib,subprocess,tempfile,time,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
def load(name):
 loader=importlib.machinery.SourceFileLoader(name,str(ROOT/'scripts'/name));spec=importlib.util.spec_from_loader(name,loader);module=importlib.util.module_from_spec(spec);loader.exec_module(module);return module
class HelpersTest(unittest.TestCase):
 def test_idle_policy_validation(self):
  agent=load('harbor-power-agent')
  self.assertEqual(agent.validate({}),{'displayMinutes':0,'suspendMinutes':0})
  args=agent.arguments({'displayMinutes':5,'suspendMinutes':15},'/usr/bin/harbor-power-agent')
  self.assertIn('300',args);self.assertIn('900',args);self.assertIn('/usr/bin/harbor-power-agent --action suspend',args)
  for bad in ({'displayMinutes':-1},{'suspendMinutes':True},{'displayMinutes':50,'suspendMinutes':10},{'displayMinutes':';reboot'}):
   with self.assertRaises(ValueError):agent.validate(bad)
 def test_idle_agent_reloads_without_touching_hardware(self):
  with tempfile.TemporaryDirectory() as d:
   root=pathlib.Path(d);config=root/'config/harbor';config.mkdir(parents=True);runtime=root/'run';runtime.mkdir();bin=root/'bin';bin.mkdir()
   stub=bin/'swayidle';stub.write_text('#!/usr/bin/python3\nimport time\nwhile True:time.sleep(1)\n');stub.chmod(0o755)
   for name in ('kscreen-doctor','systemctl'):
    command=bin/name;command.write_text('#!/usr/bin/python3\nraise SystemExit("must not execute a hardware command in this test")\n');command.chmod(0o755)
   (config/'power.json').write_text(json.dumps({'displayMinutes':5,'suspendMinutes':0}))
   env={**os.environ,'PATH':str(bin),'XDG_CONFIG_HOME':str(root/'config'),'XDG_RUNTIME_DIR':str(runtime),'WAYLAND_DISPLAY':'test-harbor-idle'};env.pop('HARBOR_PRIVATE_BUS',None)
   proc=subprocess.Popen(['/usr/bin/python3',str(ROOT/'scripts/harbor-power-agent')],env=env)
   try:
    status=None
    for _ in range(50):
     files=list((runtime/'harbor').glob('*.json'))
     if files:status=files[0];break
     time.sleep(.05)
    self.assertIsNotNone(status);data=json.loads(status.read_text());self.assertTrue(data['available']);self.assertTrue(data['running']);self.assertEqual(data['applied']['displayMinutes'],5)
    (config/'power.json').write_text(json.dumps({'displayMinutes':0,'suspendMinutes':0}))
    for _ in range(50):
     data=json.loads(status.read_text())
     if data['applied']['displayMinutes']==0:break
     time.sleep(.05)
    self.assertEqual(data['applied']['displayMinutes'],0);self.assertEqual(data['error'],'')
   finally:
    proc.terminate();proc.wait(timeout=10)
 def test_charge_helper_ignores_python_environment(self):
  with tempfile.TemporaryDirectory() as d:
   root=pathlib.Path(d);marker=root/'injected';(root/'sitecustomize.py').write_text('import pathlib\npathlib.Path('+repr(str(marker))+').touch()\n')
   env={**os.environ,'PYTHONPATH':d}
   result=subprocess.run([str(ROOT/'scripts/harbor-charge-limit'),'--help'],env=env,capture_output=True,text=True,timeout=5)
   self.assertEqual(result.returncode,0,result.stderr);self.assertFalse(marker.exists())
 def test_charge_helper_only_supported_attributes(self):
  helper=load('harbor-charge-limit')
  with tempfile.TemporaryDirectory() as d:
   root=pathlib.Path(d);battery=root/'BAT0';battery.mkdir();(battery/'type').write_text('Battery');(battery/helper.ATTR_START).write_text('40\n');(battery/helper.ATTR_END).write_text('80\n')
   self.assertEqual(helper.apply_limits('BAT0',50,90,root),{'start':50,'end':90})
   for name,start,end in (('../BAT0',40,80),('BAT0',90,80),('BAT0',-2,80),('BAT0',40,101)):
    with self.assertRaises(ValueError):helper.apply_limits(name,start,end,root)
   (battery/helper.ATTR_START).unlink();self.assertEqual(helper.apply_limits('BAT0',-1,80,root)['end'],80)
   with self.assertRaises(ValueError):helper.apply_limits('BAT0',40,80,root)
   (battery/helper.ATTR_END).unlink();(battery/helper.ATTR_END).symlink_to(root/'other')
   with self.assertRaises(ValueError):helper.apply_limits('BAT0',-1,80,root)
 def test_layout_rollback_and_confirm(self):
  with tempfile.TemporaryDirectory() as d:
   p=pathlib.Path(d);state=p/'state.json';original={'outputs':[{'id':1,'connected':True,'enabled':True,'name':'virtual','scale':1,'pos':{'x':0,'y':0},'priority':1,'currentModeId':'1','modes':[{'id':'1'},{'id':'2'}]},{'id':2,'connected':True,'enabled':True,'name':'virtual2','scale':1,'pos':{'x':1280,'y':0},'priority':2,'currentModeId':'1','modes':[{'id':'1'},{'id':'2'}]}]};state.write_text(json.dumps(original))
   fake=p/'kscreen-doctor';fake.write_text('''#!/usr/bin/python3
import json,os,pathlib,sys
p=pathlib.Path(os.environ['TEST_DISPLAY_STATE']);data=json.loads(p.read_text())
rollback=any('.priority.' in arg for arg in sys.argv[1:])
if sys.argv[1]=='-j':print(json.dumps(data))
else:
 for arg in sys.argv[1:]:
  _,ident,key,*values=arg.split('.');o=next(x for x in data['outputs'] if x['id']==int(ident));v='.'.join(values)
  if rollback and key==os.environ.get('IGNORE_ROLLBACK_FIELD'):continue
  if key=='position':x,y=map(int,v.split(','));o['pos']={'x':x,'y':y}
  elif key=='scale':o['scale']=float(v)
  elif key=='mode':o['currentModeId']=v
  elif key=='priority':o['priority']=int(v)
  elif key=='primary':
   for x in data['outputs']:x['priority']=1 if x['id']==o['id'] else 2
 p.write_text(json.dumps(data))
''');fake.chmod(0o755)
   env={**os.environ,'PATH':d,'TEST_DISPLAY_STATE':str(state)};change=json.dumps({'id':2,'x':0,'y':800,'scale':1.25,'mode':'2','primary':True})
   command=['/usr/bin/python3',str(ROOT/'scripts/harbor-display-layout-guard'),'--layout',change,'--timeout','1','--directory',str(p/'revert')]
   result=subprocess.run(command,env=env,capture_output=True,text=True,timeout=5);self.assertEqual(result.returncode,0,result.stderr);self.assertEqual(json.loads(state.read_text()),original);self.assertEqual(json.loads((p/'revert/status.json').read_text())['status'],'reverted')
   for field in ('position','scale','mode','priority'):
    with self.subTest(ignoredRollbackField=field):
     state.write_text(json.dumps(original));command[-1]=str(p/('ignored-'+field))
     result=subprocess.run(command,env={**env,'IGNORE_ROLLBACK_FIELD':field},capture_output=True,text=True,timeout=5)
     self.assertEqual(result.returncode,0,result.stderr)
     status=json.loads((p/('ignored-'+field)/'status.json').read_text())
     self.assertEqual(status['status'],'rollback-failed');self.assertIn('did not restore',status['error'])
   state.write_text(json.dumps(original))
   command[-1]=str(p/'keep');proc=subprocess.Popen(command,env=env)
   try:
    for _ in range(40):
     if (p/'keep/status.json').exists():break
     time.sleep(.02)
    (p/'keep/confirm').touch();self.assertEqual(proc.wait(timeout=5),0);self.assertEqual(json.loads((p/'keep/status.json').read_text())['status'],'confirmed');self.assertEqual(json.loads(state.read_text())['outputs'][1]['pos'],{'x':0,'y':800})
   finally:
    if proc.poll() is None:proc.kill();proc.wait()
if __name__=='__main__':unittest.main()
