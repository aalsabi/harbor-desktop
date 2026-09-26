import json
import os
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]


class SessionClientTests(unittest.TestCase):
    def run_client(self, private=False, mode='ok', omit=(), region=False):
        with tempfile.TemporaryDirectory() as directory:
            folder = pathlib.Path(directory)
            log = folder / 'events'
            fake = '''#!/usr/bin/python3
import json, os, pathlib, sys, time
name = pathlib.Path(sys.argv[0]).name
with open(os.environ['TEST_LOG'], 'a') as log:
 log.write(json.dumps({'name': name, 'args': sys.argv[1:], 'config': os.getenv('XDG_CONFIG_HOME'), 'bus': os.getenv('DBUS_SESSION_BUS_ADDRESS'), 'lang': os.getenv('LANG'), 'lc_all': os.getenv('LC_ALL')}) + '\\n')
if name == 'dbus-update-activation-environment':
 if os.environ['TEST_MODE'] == 'hang': time.sleep(30)
 if os.environ['TEST_MODE'] == 'fail': sys.exit(7)
'''
            for name in ('harbor-shell', 'dbus-update-activation-environment'):
                tool = folder / name
                tool.write_text(fake)
                tool.chmod(0o755)
            if mode == 'missing':
                (folder / 'dbus-update-activation-environment').unlink()
            env = {
                'PATH': directory,
                'TEST_LOG': str(log),
                'TEST_MODE': mode,
                'LC_CTYPE': 'C.UTF-8',
                'DBUS_SESSION_BUS_ADDRESS': 'unix:path=/test-session-bus',
                'WAYLAND_DISPLAY': 'wayland-7',
                'DISPLAY': ':7',
                'XAUTHORITY': '/test/auth',
                'XDG_CURRENT_DESKTOP': 'Harbor',
                'XDG_SESSION_DESKTOP': 'harbor',
                'XDG_SESSION_TYPE': 'wayland',
                'XDG_CONFIG_HOME': '/compositor/config',
                'HARBOR_USER_CONFIG': '/apps/config',
                'XDG_DATA_HOME': '/apps/data',
                'XDG_CACHE_HOME': '/apps/cache',
                'XDG_STATE_HOME': '/apps/state',
                'XDG_MENU_PREFIX': 'harbor-',
                'SECRET_TOKEN': 'must-not-be-exported',
            }
            if region:
                config = folder / 'config' / 'harbor'
                config.mkdir(parents=True)
                (config / 'region.json').write_text(
                    json.dumps(
                        {
                            'schema': 1,
                            'languages': ['ar', 'en'],
                            'region': 'ar_SA',
                            'formatLanguage': 'ar',
                            'measurement': 'metric',
                        }
                    )
                )
                env['HARBOR_USER_CONFIG'] = str(config.parent)
                env['LC_ALL'] = 'en_US.UTF-8'
                locale = folder / 'locale'
                locale.write_text("#!/bin/sh\nprintf 'C\\nC.utf8\\nar_SA.utf8\\n'\n")
                locale.chmod(0o755)
            if private:
                env['HARBOR_PRIVATE_BUS'] = '1'
            for key in omit:
                env.pop(key, None)
            # Prevent starting a real host authentication agent during the test.
            runner = (
                "import runpy; from unittest.mock import patch;\nwith patch('os.path.isfile', return_value=False): runpy.run_path("
                + repr(str(ROOT / 'scripts/harbor-session-client'))
                + ", run_name='__main__')"
            )
            result = subprocess.run(
                ['/usr/bin/python3', '-c', runner], env=env, capture_output=True, text=True, timeout=8
            )
            events = [json.loads(line) for line in log.read_text().splitlines()]
            return result, events, env

    def test_region_preferences_reach_activation_and_shell_without_lc_all_override(self):
        result, events, _ = self.run_client(region=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('LC_ALL=', events[0]['args'])
        for key in (
            'LANG',
            'LANGUAGE',
            'LC_MESSAGES',
            'LC_TIME',
            'LC_NUMERIC',
            'LC_MONETARY',
            'LC_MEASUREMENT',
        ):
            self.assertIn(key, events[0]['args'])
        for event in events:
            self.assertEqual(event['lang'], 'ar_SA.utf8')
            self.assertIsNone(event['lc_all'])

    def test_shared_session_updates_activation_before_shell(self):
        result, events, env = self.run_client()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual([e['name'] for e in events], ['dbus-update-activation-environment', 'harbor-shell'])
        update = events[0]
        self.assertEqual(update['config'], '/apps/config')
        self.assertEqual(update['bus'], env['DBUS_SESSION_BUS_ADDRESS'])
        self.assertEqual(
            set(update['args']),
            {
                '--systemd',
                'LC_CTYPE',
                'WAYLAND_DISPLAY',
                'DISPLAY',
                'XAUTHORITY',
                'XDG_CURRENT_DESKTOP',
                'XDG_SESSION_DESKTOP',
                'XDG_SESSION_TYPE',
                'XDG_CONFIG_HOME',
                'XDG_DATA_HOME',
                'XDG_CACHE_HOME',
                'XDG_STATE_HOME',
                'XDG_MENU_PREFIX',
                'QT_ACCESSIBILITY',
            },
        )

    def test_private_bus_never_updates_systemd_and_omits_absent_keys(self):
        result, events, env = self.run_client(private=True, omit=('DISPLAY', 'XAUTHORITY'))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(events[0]['name'], 'dbus-update-activation-environment')
        self.assertNotIn('--systemd', events[0]['args'])
        self.assertNotIn('DISPLAY', events[0]['args'])
        self.assertNotIn('XAUTHORITY', events[0]['args'])
        self.assertIn('WAYLAND_DISPLAY', events[0]['args'])
        self.assertEqual(events[0]['config'], '/apps/config')

    def test_update_failure_or_timeout_does_not_prevent_shell(self):
        for mode in ('fail', 'hang', 'missing'):
            with self.subTest(mode=mode):
                result, events, _ = self.run_client(mode=mode)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(events[-1]['name'], 'harbor-shell')
                self.assertIn('activation environment', result.stderr)


if __name__ == '__main__':
    unittest.main()
