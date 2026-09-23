#!/usr/bin/python3
"""Reproducible idle shell-only measurement; not compositor or hardware performance."""
import json,os,pathlib,platform,subprocess,tempfile,time
binary=os.environ['HARBOR_BINARY'];hz=os.sysconf('SC_CLK_TCK');samples=[]
with tempfile.TemporaryDirectory() as d,open(os.devnull,'w') as log:
 env={**os.environ,'QT_QPA_PLATFORM':'offscreen','QT_QUICK_BACKEND':'software','XDG_CONFIG_HOME':d}
 p=subprocess.Popen([binary,'--preview'],env=env,stdout=log,stderr=log)
 try:
  time.sleep(5)
  def read():
   stat=pathlib.Path(f'/proc/{p.pid}/stat').read_text().split(') ',1)[1].split()
   rss=int(next(s.split()[1] for s in pathlib.Path(f'/proc/{p.pid}/status').read_text().splitlines() if s.startswith('VmRSS:')))
   return (int(stat[11])+int(stat[12]))/hz,rss
  start_cpu,_=read();start=time.monotonic()
  for _ in range(60):
   time.sleep(1);cpu,rss=read();samples.append(rss)
  duration=time.monotonic()-start
  print(json.dumps({'scope':'Harbor preview process only; offscreen software rendering; no KWin/GPU','kernel':platform.release(),'machine':platform.machine(),'durationSeconds':duration,'warmupSeconds':5,'samples':len(samples),'rssMeanMiB':sum(samples)/len(samples)/1024,'rssPeakMiB':max(samples)/1024,'cpuPercentOfOneCore':100*(cpu-start_cpu)/duration,'hardwareValidated':False},indent=2))
 finally:p.terminate();p.wait(timeout=5)
