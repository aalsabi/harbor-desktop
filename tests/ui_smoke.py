import os,pathlib,subprocess,sys,tempfile,unittest
BIN=pathlib.Path(os.environ.get('HARBOR_BINARY','../../work/build/harbor-shell')).resolve()
class UI(unittest.TestCase):
 def test_preview_renders(self):
  self.assertTrue(BIN.exists(),'shell executable is not built')
  with tempfile.TemporaryDirectory() as d:
   env={**os.environ,'QT_QPA_PLATFORM':'offscreen','QT_QUICK_BACKEND':'software','XDG_CONFIG_HOME':d}
   r=subprocess.run([str(BIN),'--preview','--screenshot',d+'/preview.png'],env=env,capture_output=True,text=True,timeout=15)
   self.assertEqual(r.returncode,0,r.stderr)
   self.assertTrue(pathlib.Path(d+'/preview.png').stat().st_size>10000)
if __name__=='__main__':unittest.main()
