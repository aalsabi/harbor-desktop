#!/usr/bin/python3
"""Exercise the installed privileged helper only in the disposable container."""
import json, os, pathlib, subprocess, xml.etree.ElementTree as ET
assert pathlib.Path('/.dockerenv').exists() and os.geteuid() == 0
helper = '/usr/libexec/harbor-generate-locales'
requested = ['ar_SA.UTF-8', 'en_US.UTF-8']
def run(*args):
    return subprocess.run(args, capture_output=True, text=True, timeout=180)
def read_optional(path):
    p = pathlib.Path(path)
    return p.read_bytes() if p.exists() else None
before = read_optional('/etc/default/locale')
result = run(helper, *requested)
assert result.returncode == 0, result.stdout + result.stderr
assert json.loads(result.stdout)['ok']
locales = run('/usr/bin/locale', '-a')
assert locales.returncode == 0
available = {line.lower().replace('-', '') for line in locales.stdout.splitlines()}
assert all(name.lower().replace('-', '') in available for name in requested)
config = pathlib.Path('/etc/locale.gen').read_text()
assert all(name + ' UTF-8' in config.splitlines() for name in requested)
again = run(helper, *requested)
assert again.returncode == 0, again.stdout + again.stderr
assert json.loads(again.stdout)['generated'] == []
assert set(json.loads(again.stdout)['skipped']) == set(requested)
invalid = run(helper, '../../tmp/invalid')
assert invalid.returncode != 0 and not json.loads(invalid.stdout)['ok']
denied = run('/usr/sbin/runuser', '-u', 'nobody', '--', helper, *requested)
assert denied.returncode != 0 and not json.loads(denied.stdout)['ok']
assert read_optional('/etc/default/locale') == before
policy = ET.parse('/usr/share/polkit-1/actions/org.harbor.locale.policy').getroot()
action = policy.find('action')
assert action.attrib['id'] == 'org.harbor.locale.generate'
assert action.find('defaults/allow_active').text == 'auth_admin'
assert action.find('annotate').text == helper
print(json.dumps({'installedLocaleGeneration': True, 'verifiedUTF8': requested,
                 'persistentLocaleGen': True, 'idempotent': True,
                 'invalidAndUnprivilegedRejected': True, 'defaultLocaleUnchanged': True}))
