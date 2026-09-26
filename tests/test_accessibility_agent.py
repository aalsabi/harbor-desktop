import json, os, pathlib, subprocess, tempfile, time, unittest, runpy

SCRIPT = pathlib.Path(__file__).resolve().parents[1] / 'scripts/harbor-accessibility-agent'


class AccessibilityAgentTest(unittest.TestCase):
    def lease(self, values):
        calls = []

        def run(args):
            calls.append(args)
            key = tuple(args[1:3])
            if args[0] == 'get':
                return values[key]
            values[key] = args[3]
            return ''

        cls = runpy.run_path(str(SCRIPT))['PreferenceLease']
        return cls(run), calls

    def test_restores_only_preferences_it_enabled(self):
        keys = runpy.run_path(str(SCRIPT))['PreferenceLease'].KEYS
        values = {keys[0]: 'false', keys[1]: 'true'}
        lease, calls = self.lease(values)
        self.assertEqual(lease.enable(), [])
        self.assertEqual(list(values.values()), ['true', 'true'])
        self.assertEqual(lease.restore(), [])
        self.assertEqual(list(values.values()), ['false', 'true'])
        self.assertFalse(any(c[0] == 'set' and tuple(c[1:3]) == keys[1] for c in calls))

    def test_preserves_external_change(self):
        keys = runpy.run_path(str(SCRIPT))['PreferenceLease'].KEYS
        values = {key: 'false' for key in keys}
        lease, calls = self.lease(values)
        lease.enable()
        values[keys[0]] = 'false'
        calls.clear()
        lease.restore()
        self.assertFalse(any(c[0] == 'set' and tuple(c[1:3]) == keys[0] for c in calls))
        self.assertEqual(list(values.values()), ['false', 'false'])

    def test_failed_enable_not_restored(self):
        cls = runpy.run_path(str(SCRIPT))['PreferenceLease']

        def denied(args):
            if args[0] == 'get':
                return 'false'
            raise ValueError('denied')

        lease = cls(denied)
        self.assertEqual(len(lease.enable()), 2)
        self.assertEqual(lease.original, {})

    def test_owned_reader_lifecycle(self):
        for stop_by_exit in (False, True):
            with self.subTest(stop_by_exit=stop_by_exit), tempfile.TemporaryDirectory() as d:
                base = pathlib.Path(d)
                config = base / 'config/harbor'
                config.mkdir(parents=True)
                tools = base / 'bin'
                tools.mkdir()
                pidfile = base / 'reader.pid'
                statefile = base / 'gsettings.json'
                keys = runpy.run_path(str(SCRIPT))['PreferenceLease'].KEYS
                original = {
                    schema + '/' + key: ('true' if i else 'false') for i, (schema, key) in enumerate(keys)
                }
                statefile.write_text(json.dumps(original))
                (config / 'accessibility.ini').write_text('[General]\nscreenReaderEnabled=true\n')
                getter = tools / 'gsettings'
                getter.write_text(
                    "#!/usr/bin/python3\nimport os,json,sys,pathlib\np=pathlib.Path(os.environ['TEST_STATE']);values=json.loads(p.read_text());key=sys.argv[2]+'/'+sys.argv[3]\nif sys.argv[1]=='get':print(values[key])\nelse:values[key]=sys.argv[4];p.write_text(json.dumps(values))\n"
                )
                getter.chmod(0o700)
                reader = tools / 'orca'
                reader.write_text('#!/bin/sh\necho $$ > "$TEST_PID"\nexec /bin/sleep 30\n')
                reader.chmod(0o700)
                env = {
                    **os.environ,
                    'XDG_CONFIG_HOME': str(base / 'config'),
                    'PATH': str(tools),
                    'TEST_PID': str(pidfile),
                    'TEST_STATE': str(statefile),
                }
                p = subprocess.Popen(
                    ['/usr/bin/python3', str(SCRIPT)], env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE
                )
                try:
                    deadline = time.monotonic() + 4
                    while not pidfile.exists() and time.monotonic() < deadline:
                        time.sleep(0.05)
                    self.assertTrue(pidfile.exists())
                    pid = int(pidfile.read_text())
                    self.assertEqual(set(json.loads(statefile.read_text()).values()), {'true'})
                    if stop_by_exit:
                        p.terminate()
                        p.communicate(timeout=5)
                    else:
                        (config / 'accessibility.ini').write_text('[General]\nscreenReaderEnabled=false\n')
                        deadline = time.monotonic() + 4
                        while time.monotonic() < deadline:
                            f = config / 'accessibility-status.json'
                            if f.exists() and json.loads(f.read_text())['status'] == 'Screen reader off':
                                break
                            time.sleep(0.05)
                        self.assertEqual(json.loads(f.read_text())['status'], 'Screen reader off')
                    self.assertEqual(json.loads(statefile.read_text()), original)
                    with self.assertRaises(ProcessLookupError):
                        os.kill(pid, 0)
                finally:
                    if p.poll() is None:
                        p.terminate()
                    p.communicate(timeout=5)


if __name__ == '__main__':
    unittest.main()
