import json, os, pathlib, subprocess, tempfile, time, unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]


class DisplayTest(unittest.TestCase):
    def test_unconfirmed_change_reverts(self):
        script = ROOT / 'scripts/harbor-display-guard'
        self.assertTrue(script.exists(), 'display transaction helper is missing')
        with tempfile.TemporaryDirectory() as d:
            p = pathlib.Path(d)
            state = p / 'scale'
            state.write_text('1')
            doctor = p / 'kscreen-doctor'
            doctor.write_text('''#!/usr/bin/python3
import sys,json,os,pathlib
p=pathlib.Path(os.environ["TEST_SCALE"])
if sys.argv[1]=="-j":print(json.dumps({"outputs":[{"id":1,"name":"virtual","connected":True,"enabled":True,"scale":float(p.read_text()),"currentModeId":"1","pos":{"x":0,"y":0},"modes":[{"id":"1","size":{"width":1280,"height":800},"refreshRate":60}]}]}))
else:p.write_text(sys.argv[1].split(".scale.")[1])
''')
            doctor.chmod(0o755)
            env = {**os.environ, 'PATH': d, 'TEST_SCALE': str(state)}
            r = subprocess.run(
                [
                    '/usr/bin/python3',
                    str(script),
                    '--directory',
                    str(p / 'transaction'),
                    '--output',
                    '1',
                    '--scale',
                    '1.5',
                    '--timeout',
                    '1',
                ],
                env=env,
                capture_output=True,
                text=True,
                timeout=5,
            )
            self.assertEqual(r.returncode, 0, r.stderr)
            self.assertEqual(float(state.read_text()), 1)
            self.assertEqual(json.loads((p / 'transaction/status.json').read_text())['status'], 'reverted')


if __name__ == '__main__':
    unittest.main()
