import importlib.machinery,importlib.util,pathlib,tempfile,unittest
source=pathlib.Path(__file__).resolve().parents[1]/'scripts/harbor-keyboard'
class KeyboardTests(unittest.TestCase):
 def load(self):
  loader=importlib.machinery.SourceFileLoader('keyboard_helper',str(source));spec=importlib.util.spec_from_loader(loader.name,loader);mod=importlib.util.module_from_spec(spec);loader.exec_module(mod);return mod
 def test_preserves_unrelated_options_and_roundtrips_layouts(self):
  m=self.load()
  with tempfile.TemporaryDirectory() as d:
   path=pathlib.Path(d)/'kxkbrc';path.write_text('[Layout]\nOptions=caps:escape,grp:win_space_toggle\nModel=pc105\n[Other]\nValue=keep\n')
   m.save(path,['us','ara'],'grp:alt_shift_toggle',{'us','ara'})
   state=m.read(path);self.assertEqual(state['layouts'],['us','ara']);self.assertEqual(state['shortcut'],'grp:alt_shift_toggle');self.assertIn('caps:escape',path.read_text());self.assertIn('Value = keep',path.read_text())
 def test_rejects_invalid_or_empty_without_writing(self):
  m=self.load()
  with tempfile.TemporaryDirectory() as d:
   path=pathlib.Path(d)/'kxkbrc'
   for layouts,shortcut in [([],''),(['no-such-layout'],''),(['us'],'bad'),(['us']*5,'')]:
    with self.assertRaises(ValueError):m.save(path,layouts,shortcut,{'us'})
    self.assertFalse(path.exists())
 def test_preserves_existing_variant(self):
  m=self.load()
  with tempfile.TemporaryDirectory() as d:
   path=pathlib.Path(d)/'kxkbrc';path.write_text('[Layout]\nLayoutList=us,ara\nVariantList=intl,\n')
   m.save(path,['ara','us'],'',{'us','ara'});self.assertEqual(m.read(path)['variants'],['','intl'])

 def test_ambiguous_existing_variants_are_not_overwritten(self):
  m=self.load()
  with tempfile.TemporaryDirectory() as d:
   path=pathlib.Path(d)/'kxkbrc';original='[Layout]\nLayoutList=us,us\nVariantList=,intl\n';path.write_text(original)
   with self.assertRaises(ValueError):m.save(path,['us'],'',{'us'})
   self.assertEqual(path.read_text(),original)
