#!/usr/bin/python3
"""Run under KWin --virtual --output-count 2; inspect actual Wayland routing."""
import collections,json,os,re,subprocess
result=subprocess.run([os.environ.get('HARBOR_BINARY','/build/harbor-shell'),'--diagnose'],
 env={**os.environ,'WAYLAND_DEBUG':'1'},capture_output=True,text=True,timeout=20)
assert result.returncode==0,(result.returncode,result.stderr[-2000:])
assert json.loads(result.stdout)['screens']==2,result.stdout
routes=collections.defaultdict(list)
for output,role in re.findall(r'get_layer_surface\([^\n]*?, wl_output[#@](\d+), \d+, "harbor-(desktop|menubar|dock)"\)',result.stderr):
 routes[role].append(output)
for role in ('desktop','menubar','dock'):
 assert len(routes[role])==2 and len(set(routes[role]))==2,dict(routes)
assert set(routes['desktop'])==set(routes['menubar'])==set(routes['dock']),dict(routes)
print(json.dumps({'twoScreenRouting':dict(routes),'passed':True}),flush=True)
