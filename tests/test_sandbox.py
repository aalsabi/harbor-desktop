import os,pathlib,runpy,tempfile,unittest,socket,subprocess,shutil
from unittest.mock import patch
ROOT=pathlib.Path(__file__).resolve().parents[1]
class SandboxTest(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.base=pathlib.Path(self.tmp.name)
  self.env=patch.dict(os.environ,{'HOME':str(self.base/'home'),'XDG_CONFIG_HOME':str(self.base/'config'),'XDG_DATA_HOME':str(self.base/'data'),'XDG_DATA_DIRS':str(self.base/'share'),'XDG_RUNTIME_DIR':str(self.base/'runtime'),'WAYLAND_DISPLAY':'wayland-test'});self.env.start();self.addCleanup(self.env.stop)
  (self.base/'runtime').mkdir();(self.base/'home/Documents').mkdir(parents=True);self.sock=socket.socket(socket.AF_UNIX);self.sock.bind(str(self.base/'runtime/wayland-test'));self.addCleanup(self.sock.close)
  self.m=runpy.run_path(str(ROOT/'scripts/harbor-sandbox'))
 def test_exec_fields_and_rejection(self):
  self.assertEqual(self.m['expand_exec']('/usr/bin/true "a b" %% %U',{'Name':'App'},'/usr/share/applications/app.desktop'),['/usr/bin/true','a b','%'])
  with self.assertRaises(ValueError):self.m['expand_exec']('/usr/bin/true %Z',{},'/x')
  with self.assertRaises(ValueError):self.m['expand_exec']('/home/user/run',{},'/x')
 def test_closed_mounts_and_environment(self):
  cmd=self.m['build_command']('app.desktop',['/usr/bin/true'],self.m['defaults']())
  self.assertIn('--unshare-all',cmd);self.assertIn('--new-session',cmd);self.assertIn('--clearenv',cmd);self.assertNotIn('--share-net',cmd)
  self.assertNotIn('DBUS_SESSION_BUS_ADDRESS',cmd);self.assertNotIn('DISPLAY',cmd)
  self.assertNotIn(str(self.base/'home'),cmd);self.assertNotIn('/run',cmd)
  self.assertIn('--die-with-parent',cmd)
 def test_grants_and_validation(self):
  p=self.m['defaults']();p['network']=True;p['documents']='read';cmd=self.m['build_command']('app.desktop',['/usr/bin/true'],p)
  self.assertIn('--share-net',cmd);i=cmd.index(str(self.base/'home/Documents'));self.assertEqual(cmd[i-1],'--ro-bind')
  p['documents']='invalid'
  with self.assertRaises(ValueError):self.m['validate'](p)
 def test_symlink_folder_refused(self):
  (self.base/'home/Documents').rmdir();(self.base/'home/Documents').symlink_to('/etc');p=self.m['defaults']();p['documents']='write'
  with self.assertRaises(ValueError):self.m['build_command']('app.desktop',['/usr/bin/true'],p)
 def test_host_etc_not_exposed_wholesale(self):
  cmd=self.m['build_command']('app.desktop',['/usr/bin/true'],self.m['defaults']())
  triples=list(zip(cmd,cmd[1:],cmd[2:]))
  self.assertNotIn(('--ro-bind','/etc','/etc'),triples)
 def test_storage_parent_link_rejected_before_creation(self):
  (self.base/'data/harbor').mkdir(parents=True);outside=self.base/'outside';outside.mkdir()
  (self.base/'data/harbor/sandboxes').symlink_to(outside)
  with self.assertRaises(ValueError):self.m['build_command']('app.desktop',['/usr/bin/true'],self.m['defaults']())
  self.assertEqual(list(outside.iterdir()),[])
 def test_real_bwrap_containment(self):
  if not shutil.which('bwrap'):self.skipTest('bubblewrap is unavailable')
  probe='test "$HOME" = /home/harbor && test -z "$DISPLAY$DBUS_SESSION_BUS_ADDRESS" && test ! -e /etc/passwd && test ! -e /run/dbus/system_bus_socket && test ! -e /tmp/.X11-unix && test ! -e '+str(self.base/'home')+' && printf isolated > "$HOME/probe"'
  cmd=self.m['build_command']('probe.desktop',['/usr/bin/sh','-c',probe],self.m['defaults']())
  result=subprocess.run(cmd,capture_output=True,text=True,timeout=10)
  if result.returncode and ('Operation not permitted' in result.stderr or 'No permissions to create' in result.stderr):self.skipTest('Host disallows bubblewrap namespaces: '+result.stderr.strip())
  self.assertEqual(result.returncode,0,result.stderr)
  storage=self.base/'data/harbor/sandboxes'/self.m['key']('probe.desktop')
  self.assertEqual((storage/'probe').read_text(),'isolated')
 def test_child_output_is_not_collected(self):
  launch=self.m['launch'];messages=[]
  with patch.dict(launch.__globals__,{'apps':lambda:{'app.desktop':('App',['/usr/bin/true'])},'status':lambda *args:messages.append(args)}),patch('subprocess.Popen') as popen:
   popen.return_value.wait.return_value=1
   self.assertEqual(launch('app.desktop'),1)
   self.assertEqual(popen.call_args.kwargs['stdout'],subprocess.DEVNULL)
   self.assertEqual(popen.call_args.kwargs['stderr'],subprocess.DEVNULL)
 def test_unknown_app_never_launches(self):
  with self.assertRaises(ValueError):self.m['save']({'id':'../../escape','policy':self.m['defaults']()})
if __name__=='__main__':unittest.main()
