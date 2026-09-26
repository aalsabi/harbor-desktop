#!/usr/bin/python3
"""Offline VPN import tests: never instantiate NM.Client or write host settings."""
import pathlib
import runpy
import tempfile
import unittest

helper = runpy.run_path(str(pathlib.Path(__file__).resolve().parents[1] / 'scripts/harbor-network-import'))


class ImportTest(unittest.TestCase):
    def test_wireguard_in_memory_restricted_before_save(self):
        try:
            import gi
            gi.require_version('NM', '1.0')
            from gi.repository import NM
        except (ImportError, ValueError):
            self.skipTest('libnm introspection is unavailable')
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'harbor-test.conf'
            # Synthetic test keys only; this connection is never activated or saved.
            path.write_text('[Interface]\nPrivateKey = AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=\nAddress = 10.99.0.2/24\n[Peer]\nPublicKey = AQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQE=\nAllowedIPs = 10.99.0.0/24\nEndpoint = 127.0.0.1:51820\n')
            connection = helper['prepare_connection']('wireguard', str(path), NM, 'fixture-user')
            setting = connection.get_setting_connection()
            self.assertFalse(setting.get_autoconnect())
            self.assertEqual(setting.get_num_permissions(), 1)
            self.assertEqual(list(setting.get_property('permissions')), ['user:fixture-user:'])
            self.assertTrue(connection.get_setting_by_name('wireguard').get_property('private-key'))

    def test_invalid_path_before_import(self):
        with self.assertRaises(ValueError):
            helper['prepare_connection']('openvpn', 'relative.ovpn', None, 'test')
        with self.assertRaises(ValueError):
            helper['prepare_connection']('unknown', '/tmp/nonexistent.conf', None, 'test')


if __name__ == '__main__':
    unittest.main()
