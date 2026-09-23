import pathlib,subprocess,tempfile,unittest
SCRIPT=pathlib.Path(__file__).resolve().parents[1]/'scripts/harbor-file-operation'
class FileOperations(unittest.TestCase):
 def runop(self,op,dst,*sources,name=''):
  return subprocess.run([str(SCRIPT),op,'--destination',str(dst),'--name',name,'--',*map(str,sources)],capture_output=True,text=True)
 def test_copy_tree_and_preserve_symlink(self):
  with tempfile.TemporaryDirectory() as t:
   p=pathlib.Path(t);src=p/'source';src.mkdir();(src/'data').write_text('hello');(src/'link').symlink_to('data');dst=p/'target';dst.mkdir()
   r=self.runop('copy',dst,src);self.assertEqual(r.returncode,0,r.stderr);self.assertTrue((dst/'source/link').is_symlink());self.assertEqual((dst/'source/data').read_text(),'hello')
 def test_move_never_overwrites(self):
  with tempfile.TemporaryDirectory() as t:
   p=pathlib.Path(t);(p/'a').mkdir();(p/'b').mkdir();src=p/'a/file';src.write_text('new');target=p/'b/file';target.write_text('keep')
   self.assertNotEqual(self.runop('move',target.parent,src).returncode,0);self.assertEqual(target.read_text(),'keep');self.assertTrue(src.exists())
 def test_rename_and_reject_path_escape(self):
  with tempfile.TemporaryDirectory() as t:
   p=pathlib.Path(t);src=p/'a';src.write_text('test');self.assertNotEqual(self.runop('rename',p,src,name='../escape').returncode,0);self.assertEqual(self.runop('rename',p,src,name='b').returncode,0);self.assertEqual((p/'b').read_text(),'test')
 def test_reject_copy_into_self(self):
  with tempfile.TemporaryDirectory() as t:
   p=pathlib.Path(t);self.assertNotEqual(self.runop('copy',p,p).returncode,0)

 def test_fifo_copy_is_rejected(self):
  import os
  with tempfile.TemporaryDirectory() as t:
   p=pathlib.Path(t);src=p/'pipe';os.mkfifo(src);dst=p/'out';dst.mkdir()
   result=subprocess.run([str(SCRIPT),'copy','--destination',str(dst),'--',str(src)],capture_output=True,text=True,timeout=3)
   self.assertNotEqual(result.returncode,0);self.assertFalse((dst/'pipe').exists())
