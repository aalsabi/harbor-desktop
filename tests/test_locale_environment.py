import contextlib
import io
import json
import pathlib
import runpy
import tempfile
import unittest
from unittest.mock import patch

ROOT = pathlib.Path(__file__).resolve().parents[1]
HELPER = ROOT / 'scripts/harbor-locale-environment'


class LocaleEnvironmentTests(unittest.TestCase):
    def apply(self, settings, available, initial=None):
        self.assertTrue(HELPER.exists(), 'locale environment helper is missing')
        with tempfile.TemporaryDirectory() as directory:
            config = pathlib.Path(directory) / 'harbor'
            config.mkdir()
            if settings is not None:
                (config / 'region.json').write_text(
                    settings if isinstance(settings, str) else json.dumps(settings)
                )
            env = {'XDG_CONFIG_HOME': directory, 'LANG': 'en_US.UTF-8', **(initial or {})}
            before = env.copy()
            stderr = io.StringIO()
            with contextlib.redirect_stderr(stderr):
                applied = runpy.run_path(str(HELPER))['apply_locale_environment'](env, available)
            return applied, env, before, stderr.getvalue()

    def settings(self, **changes):
        return {
            'schema': 1,
            'languages': ['ar', 'en'],
            'region': 'ar_SA',
            'formatLanguage': 'ar',
            'measurement': 'metric',
            **changes,
        }

    def test_generated_locales_apply_ordered_languages_and_categories(self):
        applied, env, _, warning = self.apply(
            self.settings(), ['C', 'C.utf8', 'ar_SA.utf8', 'en_US.utf8'], {'LC_ALL': 'en_US.UTF-8'}
        )
        self.assertTrue(applied)
        self.assertEqual(env['LANG'], 'ar_SA.utf8')
        self.assertEqual(env['LANGUAGE'], 'ar:en')
        self.assertEqual(env['LC_MESSAGES'], 'ar_SA.utf8')
        for key in ('LC_TIME', 'LC_NUMERIC', 'LC_MONETARY', 'LC_MEASUREMENT'):
            self.assertEqual(env[key], 'ar_SA.utf8')
        self.assertNotIn('LC_ALL', env)
        self.assertEqual(warning, '')

    def test_language_and_region_are_independent(self):
        applied, env, _, warning = self.apply(
            self.settings(languages=['en', 'ar']), ['C', 'en_SA.utf8', 'ar_SA.utf8']
        )
        self.assertTrue(applied)
        self.assertEqual(env['LANG'], 'en_SA.utf8')
        self.assertEqual(env['LC_TIME'], 'ar_SA.utf8')
        self.assertEqual(env['LANGUAGE'], 'en:ar')

    def test_english_interface_and_all_saudi_regional_categories(self):
        applied, env, _, warning = self.apply(
            self.settings(languages=['en']),
            ['C', 'C.utf8', 'en_US.utf8', 'ar_SA.utf8'],
            {'LC_PAPER': 'en_US.utf8', 'LC_ADDRESS': 'en_US.utf8', 'LC_ALL': 'ar_SA.utf8'},
        )
        self.assertTrue(applied)
        self.assertEqual(env['LANG'], 'en_US.utf8')
        self.assertEqual(env['LANGUAGE'], 'en')
        self.assertEqual(warning, '')
        self.assertEqual(env['LC_MESSAGES'], 'en_US.utf8')
        for category in (
            'LC_CTYPE',
            'LC_COLLATE',
            'LC_TIME',
            'LC_NUMERIC',
            'LC_MONETARY',
            'LC_MEASUREMENT',
            'LC_PAPER',
            'LC_NAME',
            'LC_ADDRESS',
            'LC_TELEPHONE',
            'LC_IDENTIFICATION',
        ):
            self.assertEqual(env[category], 'ar_SA.utf8')
        self.assertNotIn('LC_ALL', env)

    def test_unavailable_locales_fall_back_to_available_inherited_locale_with_warning(self):
        applied, env, _, warning = self.apply(
            self.settings(), ['C', 'C.utf8', 'en_US.utf8'], {'LC_ALL': 'en_US.UTF-8'}
        )
        self.assertTrue(applied)
        self.assertEqual(env['LANG'], 'en_US.utf8')
        self.assertEqual(env['LC_TIME'], 'en_US.utf8')
        self.assertIn('not generated', warning)
        self.assertNotIn('LC_ALL', env)

    def test_no_generated_requested_or_inherited_locale_uses_c_utf8(self):
        _, env, _, warning = self.apply(self.settings(), ['C', 'C.UTF-8'])
        self.assertEqual(env['LANG'], 'C.UTF-8')
        self.assertIn('not generated', warning)

    def test_invalid_or_missing_settings_preserve_environment(self):
        for settings in (
            None,
            '{bad',
            [],
            self.settings(schema=2),
            self.settings(languages=['en;touch /tmp/no']),
            self.settings(region='../../x'),
            self.settings(languages=[]),
            self.settings(measurement='other'),
        ):
            with self.subTest(settings=settings):
                applied, env, before, _ = self.apply(settings, ['C'], {'LC_ALL': 'existing'})
                self.assertFalse(applied)
                self.assertEqual(env, before)

    def test_measurement_uses_generated_locale_for_selected_system(self):
        for system, expected in (('us', 'en_US.utf8'), ('uk', 'en_GB.utf8')):
            with self.subTest(system=system):
                _, env, _, warning = self.apply(
                    self.settings(measurement=system), ['C', 'ar_SA.utf8', 'en_US.utf8', 'en_GB.utf8']
                )
                self.assertEqual(env['LC_MEASUREMENT'], expected)
                self.assertEqual(env['LC_TIME'], 'ar_SA.utf8')
                self.assertEqual(warning, '')

    def test_metric_avoids_nonmetric_region_when_generated_metric_locale_exists(self):
        _, env, _, warning = self.apply(
            self.settings(region='en_US', languages=['en']), ['C', 'en_US.utf8', 'en_GB.utf8', 'ar_SA.utf8']
        )
        self.assertEqual(env['LC_MEASUREMENT'], 'ar_SA.utf8')
        self.assertEqual(env['LC_TIME'], 'en_US.utf8')
        self.assertEqual(warning, '')

    def test_unavailable_measurement_locale_warns_and_falls_back(self):
        _, env, _, warning = self.apply(self.settings(measurement='us'), ['C', 'ar_SA.utf8'])
        self.assertEqual(env['LC_MEASUREMENT'], 'ar_SA.utf8')
        self.assertIn('measurement', warning)
        self.assertIn('not generated', warning)

    def test_non_utf8_fallback_and_stale_categories_cannot_override_language(self):
        _, env, _, warning = self.apply(
            self.settings(),
            ['C', 'C.utf8', 'fr_FR'],
            {'LC_ALL': 'fr_FR', 'LANG': 'fr_FR', 'LC_CTYPE': 'fr_FR', 'LC_COLLATE': 'fr_FR'},
        )
        self.assertEqual(env['LANG'], 'C.utf8')
        self.assertEqual(env['LC_CTYPE'], 'C.utf8')
        self.assertEqual(env['LC_COLLATE'], 'C.utf8')
        self.assertIn('not generated', warning)

    def test_locale_query_failure_is_bounded_and_preserves_settings_environment(self):
        self.assertTrue(HELPER.exists(), 'locale environment helper is missing')
        helper = runpy.run_path(str(HELPER))
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'harbor'
            path.mkdir()
            (path / 'region.json').write_text(json.dumps(self.settings()))
            env = {'XDG_CONFIG_HOME': directory, 'LANG': 'en_US.UTF-8', 'LC_ALL': 'en_US.UTF-8'}
            before = env.copy()
            with (
                patch('subprocess.run', side_effect=OSError('missing locale')) as command,
                contextlib.redirect_stderr(io.StringIO()),
            ):
                self.assertFalse(helper['apply_locale_environment'](env))
            self.assertEqual(env, before)
            self.assertLessEqual(command.call_args.kwargs['timeout'], 3)


if __name__ == '__main__':
    unittest.main()
