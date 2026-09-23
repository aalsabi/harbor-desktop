#!/usr/bin/python3
import subprocess,os,time,json,ast,sys,pathlib
binary=os.environ['HARBOR_BINARY']
shell=subprocess.Popen([binary]);children=[]
def call(method,*args):
 r=subprocess.run(['gdbus','call','--session','--dest','org.harbor.Shell','--object-path','/Shell','--method','org.harbor.Shell.'+method,*args],capture_output=True,text=True,timeout=4)
 if r.returncode:raise RuntimeError(r.stderr)
 return ast.literal_eval(r.stdout)[0] if r.stdout.strip()!='()' else None
def windows():return json.loads(call('WindowList'))
try:
 for attempt in range(40):
  try:windows();break
  except Exception:time.sleep(.2)
 for mode in ['--files','--settings']:children.append(subprocess.Popen([binary,mode]))
 for _ in range(50):
  rows=windows()
  if len(rows)>=2:break
  time.sleep(.1)
 assert len(rows)>=2,rows
 for _ in range(50):
  file_row=next((r for r in windows() if r['appId']=='org.harbor.Files' and r.get('menuService')),None)
  if file_row:break
  time.sleep(.1)
 assert file_row,windows()
 result=subprocess.run(['gdbus','call','--session','--dest',file_row['menuService'],'--object-path',file_row['menuPath'],'--method','com.canonical.dbusmenu.GetLayout','--','0','-1','[]'],capture_output=True,text=True)
 assert result.returncode==0,result.stderr
 assert all(label in result.stdout for label in ['File','Edit','View','Go','Window']),result.stdout
 ids=[r['id'] for r in rows]
 call('Activate',ids[0])
 for _ in range(80):
  if next(r for r in windows() if r['id']==ids[0])['active']:break
  # Enumeration can precede mapping/focus readiness on KWin 6.3.
  time.sleep(.1);call('Activate',ids[0])
 assert next(r for r in windows() if r['id']==ids[0])['active'], {'windows':windows(),'processes':[p.poll() for p in children]}
 call('Close',ids[1]);time.sleep(.5)
 assert ids[1] not in [r['id'] for r in windows()]
 for panel in ['control','launcher','windows','settings','notifications','input']:
  call('Show',panel);time.sleep(.2)
 r=subprocess.run(['gdbus','call','--session','--dest','org.freedesktop.Notifications','--object-path','/org/freedesktop/Notifications','--method','org.freedesktop.Notifications.Notify','Harbor test','0','','Integration test','Notification delivered','[]','{}','1000'],capture_output=True,text=True)
 assert r.returncode==0,r.stderr
 print(json.dumps({'kwinWindowEnumeration':True,'activate':True,'close':True,'panelsOpened':6,'notificationDelivered':True,'initialWindowCount':len(rows),'filesMenuExported':True}),flush=True)
finally:
 for c in children+[shell]:
  if c.poll() is None:c.terminate()
 for c in children+[shell]:
  try:c.wait(timeout=3)
  except subprocess.TimeoutExpired:c.kill()
