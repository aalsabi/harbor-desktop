#!/usr/bin/python3
"""Real KWin QML handshake on private D-Bus and virtual outputs only."""

import json, os, pathlib, shutil, signal, subprocess, sys, tempfile


def main():
    probe = pathlib.Path(os.environ.get('HARBOR_GESTURE_PROBE', '/build/gesture-kwin-probe')).resolve()
    if not probe.is_file():
        raise RuntimeError('Build gesture-kwin-probe or set HARBOR_GESTURE_PROBE')
    kwin = shutil.which('kwin_wayland')
    dbus = shutil.which('dbus-run-session')
    if not kwin or not dbus:
        raise RuntimeError('kwin_wayland and dbus-run-session are required')
    with tempfile.TemporaryDirectory(prefix='harbor-gesture-kwin-') as directory:
        root = pathlib.Path(directory)
        env = os.environ.copy()
        for key in ('DISPLAY', 'WAYLAND_DISPLAY', 'XAUTHORITY', 'DBUS_SESSION_BUS_ADDRESS'):
            env.pop(key, None)
        for key, sub in (
            ('HOME', 'home'),
            ('XDG_CONFIG_HOME', 'config'),
            ('XDG_DATA_HOME', 'data'),
            ('XDG_CACHE_HOME', 'cache'),
            ('XDG_STATE_HOME', 'state'),
            ('XDG_RUNTIME_DIR', 'runtime'),
        ):
            path = root / sub
            path.mkdir(mode=0o700)
            env[key] = str(path)
        env.update(
            {
                'HARBOR_GESTURE_INTEGRATION_PRIVATE': '1',
                'HARBOR_PRIVATE_BUS': '1',
                'QT_QPA_PLATFORM': 'offscreen',
                'KWIN_COMPOSE': 'Q',
                'QT_QUICK_BACKEND': 'software',
                'LANG': 'C.UTF-8',
            }
        )
        command = [
            dbus,
            '--',
            kwin,
            '--virtual',
            '--width',
            '1024',
            '--height',
            '768',
            '--no-lockscreen',
            '--no-global-shortcuts',
            '--no-kactivities',
            '--exit-with-session',
            str(probe),
        ]
        process = subprocess.Popen(
            command,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            start_new_session=True,
        )
        try:
            output, _ = process.communicate(timeout=35)
        finally:
            # Clean only this newly created process group, including compositor helpers.
            try:
                os.killpg(process.pid, signal.SIGTERM)
            except ProcessLookupError:
                pass
            if process.poll() is None:
                try:
                    process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    os.killpg(process.pid, signal.SIGKILL)
                    process.wait()
        print(output, flush=True)
        if process.returncode:
            raise RuntimeError('Virtual KWin exited with ' + str(process.returncode))
        evidence = next(
            (json.loads(line) for line in output.splitlines() if line.startswith('{"virtualKWin":')), None
        )
        if evidence != {'virtualKWin': True, 'readyHandshake': True, 'mappingReload': True, 'unloaded': True}:
            raise RuntimeError('The compositor did not report the complete gesture handshake')


if __name__ == '__main__':
    try:
        main()
    except Exception as error:
        sys.exit(str(error))
