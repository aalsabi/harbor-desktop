import contextlib
import importlib.machinery
import importlib.util
import pathlib
import subprocess
import tempfile
import types
import unittest
import xml.etree.ElementTree as ET
from unittest.mock import patch

ROOT = pathlib.Path(__file__).resolve().parents[1]
HELPER = ROOT / 'scripts/harbor-generate-locales'


class GenerateLocalesTests(unittest.TestCase):
    def helper(self):
        self.assertTrue(HELPER.exists(), 'privileged locale helper is missing')
        loader = importlib.machinery.SourceFileLoader('harbor_generate_locales', str(HELPER))
        spec = importlib.util.spec_from_loader(loader.name, loader)
        module = importlib.util.module_from_spec(spec)
        loader.exec_module(module)
        return module

    def test_arguments_must_exactly_match_supported_utf8_entries(self):
        helper = self.helper()
        supported = {'ar_SA.UTF-8', 'en_US.UTF-8'}
        for names in ([], ['ar_SA'], ['en_US.ISO-8859-1'], ['../../tmp/x'], ['--prefix=/tmp'],
                      ['ar_SA.UTF-8;id'], ['ar_SA.UTF-8\n'], ['ar_SA.UTF-8'] * 33):
            with self.subTest(names=names), self.assertRaises(ValueError):
                helper.validate_requests(names, supported)
        self.assertEqual(helper.validate_requests(['ar_SA.UTF-8', 'ar_SA.UTF-8'], supported), ['ar_SA.UTF-8'])

    def test_supported_file_requires_root_ownership_and_no_untrusted_write(self):
        helper = self.helper()
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'SUPPORTED'
            path.write_text('ar_SA.UTF-8 UTF-8\nen_US ISO-8859-1\nen_US.UTF-8 UTF-8\n')
            with patch.object(helper.os, 'fstat', return_value=types.SimpleNamespace(st_uid=0, st_mode=0o100644)):
                self.assertEqual(helper.read_supported(path), {'ar_SA.UTF-8', 'en_US.UTF-8'})
            for owner, mode in ((1000, 0o100644), (0, 0o100666)):
                with patch.object(helper.os, 'fstat', return_value=types.SimpleNamespace(st_uid=owner, st_mode=mode)), self.assertRaises(ValueError):
                    helper.read_supported(path)

    def test_generation_uses_fixed_commands_clean_environment_and_skips_existing(self):
        helper = self.helper()
        calls = []
        def command(args, **kwargs):
            calls.append((args, kwargs))
            output = 'C\nen_US.utf8\n' if len(calls) == 1 else 'C\nen_US.utf8\nar_SA.utf8\n'
            return subprocess.CompletedProcess(args, 0, stdout=output, stderr='')
        with patch.object(helper.os, 'geteuid', return_value=0), patch.object(helper, 'read_supported', return_value={'ar_SA.UTF-8', 'en_US.UTF-8'}), patch.object(helper, 'generation_lock', return_value=contextlib.nullcontext()), patch.object(helper, 'persist_requests') as persist, patch.object(helper.subprocess, 'run', side_effect=command):
            result = helper.generate(['ar_SA.UTF-8', 'en_US.UTF-8'])
        persist.assert_called_once_with(['ar_SA.UTF-8', 'en_US.UTF-8'])
        self.assertTrue(result['ok'])
        self.assertEqual(result['generated'], ['ar_SA.UTF-8'])
        self.assertEqual(result['skipped'], ['en_US.UTF-8'])
        self.assertEqual(calls[1][0], ['/usr/bin/localedef', '--inputfile=ar_SA', '--charmap=UTF-8', '--', 'ar_SA.UTF-8'])
        for args, options in calls:
            self.assertEqual(options['env'], {'PATH': '/usr/sbin:/usr/bin:/sbin:/bin', 'LANG': 'C', 'LC_ALL': 'C'})
            self.assertNotIn('shell', options)
            self.assertEqual(options['cwd'], '/')
            self.assertLessEqual(options['timeout'], 120)
        self.assertEqual(calls[0][0], ['/usr/bin/locale', '-a'])
        self.assertEqual(calls[-1][0], ['/usr/bin/locale', '-a'])

    def test_non_root_cannot_invoke_generation(self):
        helper = self.helper()
        with patch.object(helper.os, 'geteuid', return_value=1000), patch.object(helper.subprocess, 'run') as run, self.assertRaises(PermissionError):
            helper.generate(['ar_SA.UTF-8'])
        run.assert_not_called()

    def test_persistence_preserves_comments_existing_entries_and_mode(self):
        helper = self.helper()
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'locale.gen'
            original = '# local choices\nfr_FR.UTF-8 UTF-8\n# ar_SA.UTF-8 UTF-8\n'
            path.write_text(original)
            path.chmod(0o640)
            original_stat = path.stat()
            fake_stat = types.SimpleNamespace(st_uid=0, st_gid=original_stat.st_gid, st_mode=original_stat.st_mode, st_dev=original_stat.st_dev, st_ino=original_stat.st_ino, st_size=original_stat.st_size, st_mtime_ns=original_stat.st_mtime_ns)
            with patch.object(helper.os, 'fstat', return_value=fake_stat), patch.object(helper.os, 'fchown'):
                helper.persist_requests(['ar_SA.UTF-8', 'fr_FR.UTF-8'], path)
                first = path.read_text()
                helper.persist_requests(['ar_SA.UTF-8'], path)
            self.assertTrue(first.startswith(original))
            self.assertEqual(first.count('\nar_SA.UTF-8 UTF-8\n'), 1)
            self.assertEqual(first.count('fr_FR.UTF-8 UTF-8'), 1)
            self.assertEqual(path.read_text(), first)
            self.assertEqual(path.stat().st_mode & 0o777, 0o640)

    def test_concurrent_admin_edit_is_preserved(self):
        helper = self.helper()
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'locale.gen'
            path.write_text('# original\n')
            def edited(_):
                path.write_text('# concurrent administrator edit\nfr_FR.UTF-8 UTF-8\n')
            with patch.object(helper, 'trusted_file'), patch.object(helper.os, 'fchown'), patch.object(helper.os, 'fsync', side_effect=edited):
                with self.assertRaises(RuntimeError):
                    helper.persist_requests(['ar_SA.UTF-8'], path)
            self.assertEqual(path.read_text(), '# concurrent administrator edit\nfr_FR.UTF-8 UTF-8\n')

    def test_polkit_policy_requires_admin_for_exact_helper_path(self):
        policy = ET.parse(ROOT / 'config/polkit/org.harbor.locale.policy').getroot()
        action = policy.find('action')
        self.assertEqual(action.attrib['id'], 'org.harbor.locale.generate')
        self.assertEqual(action.findtext('defaults/allow_active'), 'auth_admin')
        self.assertEqual(action.findtext('defaults/allow_inactive'), 'no')
        self.assertEqual(action.findtext('defaults/allow_any'), 'no')
        self.assertEqual(action.find('annotate').attrib['key'], 'org.freedesktop.policykit.exec.path')
        self.assertEqual(action.findtext('annotate'), '/usr/libexec/harbor-generate-locales')
        self.assertEqual(HELPER.read_text().splitlines()[0], '#!/usr/bin/python3 -I')

    def test_command_failure_does_not_enable_unverified_locales(self):
        helper = self.helper()
        failure = subprocess.CalledProcessError(1, ['/usr/bin/localedef'], stderr='failed')
        with patch.object(helper.os, 'geteuid', return_value=0), patch.object(helper, 'read_supported', return_value={'ar_SA.UTF-8'}), patch.object(helper, 'generation_lock', return_value=contextlib.nullcontext()), patch.object(helper, 'persist_requests') as persist, patch.object(helper.subprocess, 'run', side_effect=[subprocess.CompletedProcess([], 0, 'C\n', ''), failure]):
            with self.assertRaises(subprocess.CalledProcessError):
                helper.generate(['ar_SA.UTF-8'])
            persist.assert_not_called()

    def test_verification_failure_is_not_success(self):
        helper = self.helper()
        with patch.object(helper.os, 'geteuid', return_value=0), patch.object(helper, 'read_supported', return_value={'ar_SA.UTF-8'}), patch.object(helper, 'generation_lock', return_value=contextlib.nullcontext()), patch.object(helper, 'persist_requests') as persist, patch.object(helper.subprocess, 'run', return_value=subprocess.CompletedProcess([], 0, 'C\n', '')):
            with self.assertRaises(RuntimeError):
                helper.generate(['ar_SA.UTF-8'])
            persist.assert_not_called()


if __name__ == '__main__':
    unittest.main()
