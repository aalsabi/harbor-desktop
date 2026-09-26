#!/usr/bin/python3
"""Expose a real Qt Wayland window through Bubblewrap on a private virtual KWin."""

import json, os, pathlib, runpy, shlex, shutil, subprocess, sys, tempfile, signal

ROOT = pathlib.Path(__file__).resolve().parents[1]


def client():
    probe = pathlib.Path(os.environ['HARBOR_SANDBOX_GUI_PROBE']).resolve()
    helper = runpy.run_path(str(ROOT / 'scripts/harbor-sandbox'))
    cmd = helper['build_command'](
        'gui-fixture.desktop', ['/usr/bin/harbor-gui-fixture'], helper['defaults']()
    )
    boundary = cmd.index('--')
    # A private bin directory avoids creating a mountpoint in the host's read-only /usr.
    cmd[boundary:boundary] = ['--tmpfs', '/usr/bin', '--ro-bind', str(probe), '/usr/bin/harbor-gui-fixture']
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=20)
    output = {'returncode': result.returncode, 'stderr': result.stderr[-4000:]}
    pathlib.Path(os.environ['HARBOR_GUI_RESULT']).write_text(json.dumps(output))
    return result.returncode


def main():
    probe = pathlib.Path(os.environ.get('HARBOR_SANDBOX_GUI_PROBE', '/nonexistent'))
    if not probe.is_file():
        raise RuntimeError('Set HARBOR_SANDBOX_GUI_PROBE to the compiled QtGui fixture')
    for name in ('bwrap', 'kwin_wayland', 'dbus-run-session'):
        if not shutil.which(name):
            print('SKIP: sandbox GUI test requires ' + name)
            return 77
    permission = subprocess.run(
        ['bwrap', '--unshare-all', '--ro-bind', '/', '/', '--', '/usr/bin/true'],
        capture_output=True,
        text=True,
        timeout=10,
    )
    if permission.returncode:
        if 'Operation not permitted' in permission.stderr or 'No permissions to create' in permission.stderr:
            print('SKIP: host prohibits Bubblewrap user namespaces: ' + permission.stderr.strip())
            return 77
        raise RuntimeError(permission.stderr)
    with tempfile.TemporaryDirectory(prefix='harbor-sandbox-gui-') as directory:
        base = pathlib.Path(directory)
        env = dict(os.environ)
        for key in (
            'DISPLAY',
            'WAYLAND_DISPLAY',
            'XAUTHORITY',
            'DBUS_SESSION_BUS_ADDRESS',
            'QT_QPA_PLATFORM',
            'QT_PLUGIN_PATH',
            'QT_QPA_PLATFORM_PLUGIN_PATH',
        ):
            env.pop(key, None)
        for key, folder in [
            ('HOME', 'home'),
            ('XDG_CONFIG_HOME', 'config'),
            ('XDG_DATA_HOME', 'data'),
            ('XDG_CACHE_HOME', 'cache'),
            ('XDG_RUNTIME_DIR', 'runtime'),
        ]:
            path = base / folder
            path.mkdir(mode=0o700)
            env[key] = str(path)
        env['XDG_CONFIG_DIRS'] = env['XDG_CONFIG_HOME']
        env.update(
            {
                'HARBOR_SANDBOX_GUI_PROBE': str(probe.resolve()),
                'HARBOR_GUI_RESULT': str(base / 'result.json'),
                'KWIN_COMPOSE': 'Q',
                'LIBGL_ALWAYS_SOFTWARE': '1',
                'QT_QUICK_BACKEND': 'software',
            }
        )
        script = base / 'client'
        script.write_text(
            '#!/bin/sh\nexec '
            + shlex.quote(sys.executable)
            + ' '
            + shlex.quote(str(pathlib.Path(__file__).resolve()))
            + ' --client\n'
        )
        script.chmod(0o700)
        command = [
            'dbus-run-session',
            '--',
            'kwin_wayland',
            '--virtual',
            '--width',
            '800',
            '--height',
            '600',
            '--no-lockscreen',
            '--no-global-shortcuts',
            '--no-kactivities',
            '--exit-with-session',
            str(script),
        ]
        process = subprocess.Popen(
            command,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            start_new_session=True,
        )
        try:
            stdout, stderr = process.communicate(timeout=35)
        finally:
            try:
                os.killpg(process.pid, signal.SIGTERM)
            except ProcessLookupError:
                pass
            if process.poll() is None:
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    try:
                        os.killpg(process.pid, signal.SIGKILL)
                    except ProcessLookupError:
                        pass
                    process.wait(timeout=5)
        result = base / 'result.json'
        if not result.exists():
            raise RuntimeError('Private compositor/client did not complete: ' + stderr[-5000:])
        outcome = json.loads(result.read_text())
        if outcome['returncode'] != 0:
            raise RuntimeError('Isolated GUI fixture failed: ' + json.dumps(outcome))
        import hashlib

        marker = (
            base
            / 'data/harbor/sandboxes'
            / hashlib.sha256(b'gui-fixture.desktop').hexdigest()
            / 'gui-probe.json'
        )
        evidence = json.loads(marker.read_text())
        if evidence != {'wayland': True, 'exposed': True, 'isolatedHome': True, 'hostBusAbsent': True}:
            raise RuntimeError('Incomplete isolation evidence: ' + repr(evidence))
        print(json.dumps({'privateVirtualKWin': True, 'realBubblewrap': True, **evidence}, sort_keys=True))
        return 0


if __name__ == '__main__':
    try:
        sys.exit(client() if sys.argv[1:] == ['--client'] else main())
    except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        print(str(error), file=sys.stderr)
        sys.exit(1)
