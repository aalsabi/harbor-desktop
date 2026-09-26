import os, pathlib, runpy, tempfile, unittest
from unittest.mock import patch

SCRIPT = pathlib.Path(__file__).resolve().parents[1] / 'scripts/harbor-user-settings'


class UserSettingsTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.base = pathlib.Path(self.tmp.name)
        self.env = patch.dict(
            os.environ,
            {
                'XDG_CONFIG_HOME': str(self.base / 'config'),
                'XDG_DATA_HOME': str(self.base / 'data'),
                'XDG_CONFIG_DIRS': str(self.base / 'etc'),
                'XDG_DATA_DIRS': str(self.base / 'share'),
            },
        )
        self.env.start()
        self.addCleanup(self.env.stop)
        self.mod = runpy.run_path(str(SCRIPT))
        self.g = self.mod['dispatch'].__globals__

    def entry(self, path, extra=''):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text('[Desktop Entry]\nType=Application\nName=Test\nExec=/bin/true\n' + extra)

    def test_startup_add_toggle_and_unknown(self):
        self.entry(self.base / 'data/applications/test.desktop')
        self.mod['startup_add']({'id': 'test.desktop'})
        rows = self.mod['startup']()['entries']
        self.assertTrue(rows[0]['enabled'])
        self.mod['startup_set']({'id': 'test.desktop', 'enabled': False})
        self.assertFalse(self.mod['startup']()['entries'][0]['enabled'])
        with self.assertRaises(ValueError):
            self.mod['startup_add']({'id': '../../escape.desktop'})

    def test_desktop_visibility_and_override(self):
        self.entry(self.base / 'etc/autostart/kde.desktop', 'OnlyShowIn=KDE;\n')
        self.assertFalse(self.mod['startup']()['entries'][0]['enabled'])
        self.mod['startup_set']({'id': 'kde.desktop', 'enabled': True})
        self.assertTrue(self.mod['startup']()['entries'][0]['enabled'])
        self.assertIn('OnlyShowIn=KDE', (self.base / 'etc/autostart/kde.desktop').read_text())

    def test_permission_fixed_args(self):
        calls = []

        def run(args, timeout=15):
            calls.append(args)
            if 'list' in args:
                return 'org.test.App\tTest\n'
            if 'info' in args:
                return '[Context]\nshared=network;\nsockets=pulseaudio;\nfilesystems=home;\n'
            return ''

        self.g['run'] = run
        which = patch('shutil.which', side_effect=lambda name: '/usr/bin/' + name)
        which.start()
        self.addCleanup(which.stop)
        self.mod['permission_set']({'id': 'org.test.App', 'key': 'network', 'value': False})
        self.assertIn(['flatpak', 'override', '--user', '--unshare=network', 'org.test.App'], calls)
        with self.assertRaises(ValueError):
            self.mod['permission_set']({'id': '--system', 'key': 'network', 'value': True})

    def test_package_size_normalization(self):
        self.g['run'] = lambda args: 'ii \tbig\t200\nii \tsmall\t2\nrc \tremoved\t900\n'
        which = patch('shutil.which', side_effect=lambda n: '/usr/bin/' + n)
        which.start()
        self.addCleanup(which.stop)
        rows = self.mod['package_sizes']()['applications']
        self.assertEqual([r['name'] for r in rows], ['big', 'small'])
        self.assertEqual(rows[0]['bytes'], 204800)


if __name__ == '__main__':
    unittest.main()
