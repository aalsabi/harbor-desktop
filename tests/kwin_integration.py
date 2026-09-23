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
 for _ in range(2):children.append(subprocess.Popen([binary,'--settings']))
 for _ in range(50):
  rows=windows()
  if len(rows)>=2:break
  time.sleep(.1)
 assert len(rows)>=2,rows
 ids=[r['id'] for r in rows]
 call('Activate',ids[0])
 for _ in range(30):
  if next(r for r in windows() if r['id']==ids[0])['active']:break
  time.sleep(.1)
 assert next(r for r in windows() if r['id']==ids[0])['active']
 call('Close',ids[1]);time.sleep(.5)
 assert ids[1] not in [r['id'] for r in windows()]
 for panel in ['control','launcher','windows','settings','notifications']:
  call('Show',panel);time.sleep(.2)
 r=subprocess.run(['gdbus','call','--session','--dest','org.freedesktop.Notifications','--object-path','/org/freedesktop/Notifications','--method','org.freedesktop.Notifications.Notify','Harbor test','0','','Integration test','Notification delivered','[]','{}','1000'],capture_output=True,text=True)
 assert r.returncode==0,r.stderr
 print(json.dumps({'kwinWindowEnumeration':True,'activate':True,'close':True,'panelsOpened':5,'notificationDelivered':True,'initialWindowCount':len(rows)}),flush=True)
finally:
 for c in children+[shell]:
  if c.poll() is None:c.terminate()
 for c in children+[shell]:
  try:c.wait(timeout=3)
  except subprocess.TimeoutExpired:c.kill()
