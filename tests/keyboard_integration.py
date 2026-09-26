#!/usr/bin/python3
"""Verify isolated KWin actually loads/reloads Harbor keyboard configuration."""

import os, sys, subprocess, tempfile, pathlib, time, json


def call(method, *args):
    r = subprocess.run(
        [
            'gdbus',
            'call',
            '--session',
            '--dest',
            'org.kde.keyboard',
            '--object-path',
            '/Layouts',
            '--method',
            'org.kde.KeyboardLayouts.' + method,
            *args,
        ],
        capture_output=True,
        text=True,
        timeout=3,
    )
    if r.returncode:
        raise RuntimeError(r.stderr)
    return r.stdout


if '--client' in sys.argv:
    for _ in range(50):
        try:
            layouts = call('getLayoutsList')
            if 'ara' in layouts:
                break
        except RuntimeError:
            pass
        time.sleep(0.1)
    assert 'ara' in layouts, layouts
    assert 'true' in call('setLayout', '1')
    assert '1' in call('getLayout')
    env = {**os.environ, 'XDG_CONFIG_HOME': os.environ['HARBOR_KEYBOARD_TEST_BASE']}
    subprocess.run(
        ['harbor-keyboard', '--layouts', 'us,de', '--shortcut', 'grp:alt_shift_toggle'],
        env=env,
        check=True,
        stdout=subprocess.DEVNULL,
    )
    subprocess.run(
        [
            'gdbus',
            'emit',
            '--session',
            '--object-path',
            '/Layouts',
            '--signal',
            'org.kde.keyboard.reloadConfig',
        ],
        check=True,
    )
    subprocess.run(
        [
            'gdbus',
            'emit',
            '--session',
            '--object-path',
            '/kxkbrc',
            '--signal',
            'org.kde.kconfig.notify.ConfigChanged',
            "@a{saay} {'Layout': [b'LayoutList', b'VariantList', b'Options', b'Use', b'ResetOldOptions']}",
        ],
        check=True,
    )
    for _ in range(50):
        layouts = call('getLayoutsList')
        if "'de'" in layouts:
            break
        time.sleep(0.1)
    assert "'de'" in layouts and "'ara'" not in layouts, layouts
    print(
        json.dumps({'keyboardLayoutsLoaded': True, 'keyboardSwitch': True, 'keyboardReload': True}),
        flush=True,
    )
else:
    with tempfile.TemporaryDirectory(prefix='harbor-keyboard-test-') as tmp:
        env = {**os.environ, 'XDG_CONFIG_HOME': tmp, 'HARBOR_KEYBOARD_TEST_BASE': tmp}
        subprocess.run(
            ['harbor-keyboard', '--layouts', 'us,ara', '--shortcut', 'grp:alt_shift_toggle'],
            env=env,
            check=True,
            stdout=subprocess.DEVNULL,
        )
        env['XDG_CONFIG_HOME'] = str(pathlib.Path(tmp) / 'harbor/session')
        script = pathlib.Path(tmp) / 'client'
        script.write_text(
            '#!/bin/sh\nexec /usr/bin/python3 ' + str(pathlib.Path(__file__).resolve()) + ' --client\n'
        )
        script.chmod(0o700)
        r = subprocess.run(
            [
                'kwin_wayland',
                '--virtual',
                '--no-lockscreen',
                '--no-global-shortcuts',
                '--no-kactivities',
                '--exit-with-session',
                str(script),
            ],
            env=env,
            timeout=25,
        )
        raise SystemExit(r.returncode)
