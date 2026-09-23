#!/usr/bin/python3
"""Installed headless session smoke test. Run under a fresh dbus-run-session."""
import ast,json,os,subprocess,time
p=subprocess.Popen(['harbor-session','--headless'],env={**os.environ,'HARBOR_PRIVATE_BUS':'1'})
def call(method,*args):
 r=subprocess.run(['gdbus','call','--session','--dest','org.harbor.Shell','--object-path','/Shell','--method','org.harbor.Shell.'+method,*args],capture_output=True,text=True,timeout=3)
 if r.returncode:raise RuntimeError(r.stderr)
 return ast.literal_eval(r.stdout)[0] if r.stdout.strip()!='()' else None
try:
 for _ in range(80):
  if p.poll() is not None:raise RuntimeError('session exited: '+str(p.returncode))
  try:call('WindowList');break
  except Exception:time.sleep(.1)
 call('Show','settings')
 for _ in range(40):
  rows=json.loads(call('WindowList'))
  if rows:break
  time.sleep(.1)
 assert rows, 'session could not enumerate its settings window'
 call('Logout');p.wait(timeout=8);assert p.returncode==0,p.returncode
 print(json.dumps({'installedSessionStartup':True,'settingsWindowManaged':True,'gracefulLogout':True}),flush=True)
finally:
 if p.poll() is None:p.terminate()
 p.wait(timeout=8)
