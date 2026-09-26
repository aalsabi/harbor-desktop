"""Render native settings with isolated user state and a private session bus."""

import os
import pathlib
import re
import shutil
import struct
import subprocess
import tempfile
import unittest
import zlib
from collections import Counter

ROOT = pathlib.Path(__file__).resolve().parents[1]
BIN = pathlib.Path(os.environ.get('HARBOR_BINARY', ROOT / '../../work/build/harbor-shell')).resolve()
PAGES = (
    'Date & Time',
    'Mouse',
    'Touchpad',
    'Default Applications',
    'Storage',
    'Printers',
    'Wi-Fi',
    'Network',
    'Bluetooth',
    'Users & Groups',
    'Software Update',
    'Notifications & Focus',
    'Startup Applications',
    'Privacy & Permissions',
    'Displays',
    'Sound',
    'Battery',
    'Accessibility',
    'Gestures',
)
FOCUSED_PAGES = (
    'Notifications & Focus',
    'Printers',
    'Network',
    'Bluetooth',
    'Startup Applications',
    'Privacy & Permissions',
    'Storage',
)
QML_ERRORS = re.compile(
    r'ReferenceError|TypeError|SyntaxError|is not a type|Type [^\n]+ unavailable|'
    r'Cannot assign to|Invalid property assignment|Expected token|'
    r'Binding loop detected|Unable to assign|Cannot read property|'
    r'QQmlApplicationEngine failed|QQmlComponent: Component is not ready'
)


def content_colors(png, arabic=False):
    """Decode Qt's 8-bit PNG scanlines without optional imaging dependencies."""
    width, height = struct.unpack('>II', png[16:24])
    depth, color, _compression, _filter, interlace = png[24:29]
    if depth != 8 or color not in (2, 6) or interlace:
        raise AssertionError('Unexpected screenshot PNG format')
    channels = 4 if color == 6 else 3
    compressed, offset = bytearray(), 8
    while offset < len(png):
        length = struct.unpack('>I', png[offset : offset + 4])[0]
        if png[offset + 4 : offset + 8] == b'IDAT':
            compressed.extend(png[offset + 8 : offset + 8 + length])
        offset += 12 + length
    raw = zlib.decompress(compressed)
    stride, previous, colors = width * channels, bytearray(width * channels), Counter()
    left, right = (20, width - 250) if arabic else (250, width - 20)
    for y in range(height):
        start = y * (stride + 1)
        kind, row = raw[start], bytearray(raw[start + 1 : start + 1 + stride])
        for x in range(stride):
            a, b = row[x - channels] if x >= channels else 0, previous[x]
            c = previous[x - channels] if x >= channels else 0
            if kind == 1:
                row[x] = (row[x] + a) & 255
            elif kind == 2:
                row[x] = (row[x] + b) & 255
            elif kind == 3:
                row[x] = (row[x] + (a + b) // 2) & 255
            elif kind == 4:
                predictor = a + b - c
                distances = (abs(predictor - a), abs(predictor - b), abs(predictor - c))
                base = (a, b, c)[distances.index(min(distances))]
                row[x] = (row[x] + base) & 255
            elif kind != 0:
                raise AssertionError('Unknown PNG filter')
        if 60 <= y < height - 20 and y % 3 == 0:
            for x in range(left, right, 3):
                colors[bytes(row[x * channels : x * channels + 3])] += 1
        previous = row
    return colors


class UI(unittest.TestCase):
    def render(self, arguments, name, language='en', dark=False, size=None):
        self.assertTrue(BIN.exists(), 'shell executable is not built')
        self.assertIsNotNone(shutil.which('dbus-run-session'), 'dbus-run-session is required')
        with tempfile.TemporaryDirectory(prefix='harbor-ui-') as temporary:
            home = pathlib.Path(temporary)
            for child in ('config/harbor', 'data', 'cache', 'runtime'):
                (home / child).mkdir(parents=True)
            (home / 'runtime').chmod(0o700)
            (home / 'config/harbor/settings.ini').write_text(
                f'[General]\nlanguage={language}\ndark={str(dark).lower()}\n', encoding='utf-8'
            )
            env = {
                **os.environ,
                'HOME': str(home),
                'QT_QPA_PLATFORM': 'offscreen',
                'QT_QUICK_BACKEND': 'software',
                'XDG_CONFIG_HOME': str(home / 'config'),
                'XDG_DATA_HOME': str(home / 'data'),
                'XDG_CACHE_HOME': str(home / 'cache'),
                'XDG_RUNTIME_DIR': str(home / 'runtime'),
                'DBUS_SYSTEM_BUS_ADDRESS': 'unix:path=/nonexistent-harbor-test-bus',
                'PATH': str(ROOT / 'scripts') + os.pathsep + os.environ.get('PATH', ''),
            }
            # The private session must not reuse the user's desktop bus or compositor.
            for key in ('DBUS_SESSION_BUS_ADDRESS', 'WAYLAND_DISPLAY', 'DISPLAY'):
                env.pop(key, None)
            output = home / 'page.png'
            command = ['dbus-run-session', '--', str(BIN), *arguments]
            if size:
                command += ['--screenshot-size', f'{size[0]}x{size[1]}']
            command += ['--screenshot', str(output)]
            result = subprocess.run(command, env=env, capture_output=True, text=True, timeout=20)
            diagnostic = result.stdout + '\n' + result.stderr
            self.assertEqual(result.returncode, 0, diagnostic)
            self.assertNotRegex(diagnostic, QML_ERRORS)
            self.assertTrue(output.exists(), diagnostic)
            image = output.read_bytes()
            self.assertEqual(image[:8], b'\x89PNG\r\n\x1a\n')
            self.assertGreater(len(image), 10000, diagnostic)
            dimensions = struct.unpack('>II', image[16:24])
            artifact_dir = os.environ.get('HARBOR_UI_ARTIFACT_DIR')
            if artifact_dir:
                destination = pathlib.Path(artifact_dir)
                destination.mkdir(parents=True, exist_ok=True)
                slug = re.sub(r'[^a-z0-9]+', '-', name.lower()).strip('-')
                shutil.copyfile(output, destination / f'{slug}-{language}-{"dark" if dark else "light"}.png')
            if size:
                self.assertEqual(dimensions, size, 'Requested narrow geometry was not applied')
            if '--settings' in arguments:
                colors = content_colors(image, arabic=language == 'ar')
                self.assertGreater(len(colors), 8, f'{name}: settings content is blank')
                dominant = colors.most_common(1)[0][1] / sum(colors.values())
                self.assertLess(dominant, 0.995, f'{name}: settings content is almost entirely blank')

    def test_preview_renders(self):
        self.render(['--preview'], 'preview')

    def test_native_settings_pages_render(self):
        for page in PAGES:
            with self.subTest(page=page):
                self.render(['--settings', page], page)

    def test_arabic_dark_narrow_pages_render(self):
        for page in FOCUSED_PAGES:
            with self.subTest(page=page):
                self.render(['--settings', page], page, language='ar', dark=True, size=(740, 560))


if __name__ == '__main__':
    unittest.main()
