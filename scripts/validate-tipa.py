"""Validate the installable bundle, device architecture, signature and size."""
import hashlib
import json
from pathlib import Path
import plistlib
import struct
import sys
import zipfile


def inspect_macho(data):
    # This package intentionally contains one arm64 device slice per executable.
    magic, cpu, subtype, kind, commands, size, flags, reserved = struct.unpack_from('<8I', data)
    assert magic == 0xFEEDFACF, 'Expected a thin 64-bit Mach-O executable'
    assert cpu == 0x0100000C and subtype & 0x00FFFFFF == 0, 'Expected arm64, never an Intel host binary'
    assert kind == 2, 'Expected MH_EXECUTE'
    offset = 32
    minimum = None
    signed = False
    libraries = []
    for _ in range(commands):
        command, length = struct.unpack_from('<2I', data, offset)
        assert length >= 8 and offset + length <= 32 + size
        if command == 0x32:  # LC_BUILD_VERSION
            platform, minimum = struct.unpack_from('<2I', data, offset + 8)
            assert platform == 2, 'Expected iOS device platform'
        elif command == 0x25:  # LC_VERSION_MIN_IPHONEOS
            minimum = struct.unpack_from('<I', data, offset + 8)[0]
        elif command == 0x1D:  # LC_CODE_SIGNATURE
            start, count = struct.unpack_from('<2I', data, offset + 8)
            signed = count > 0 and start + count <= len(data)
        elif command in (0xC, 0x80000018, 0x8000001F):
            start = struct.unpack_from('<I', data, offset + 8)[0]
            name = data[offset + start:offset + length].split(b'\0')[0].decode()
            assert name.startswith(('/System/Library/', '/usr/lib/', '@')), name
            libraries.append(name)
        offset += length
    assert signed, 'Missing embedded code signature'
    assert minimum is not None and minimum <= 0x000C0000, 'Deployment target must support iOS 12+'
    return {'architecture': 'arm64', 'minimum_ios': f'{minimum >> 16}.{(minimum >> 8) & 255}',
            'signed': signed, 'bytes': len(data), 'libraries': libraries}


def validate(path):
    size = path.stat().st_size
    assert size < 10 * 1024 * 1024, 'Unexpected package growth: exceeds 10 MiB'
    prefix = 'Payload/ChargeLimiter.app/'
    with zipfile.ZipFile(path) as package:
        names = package.namelist()
        assert len(names) == len(set(names)), 'Duplicate archive entries'
        assert all(n == 'Payload/' or n.startswith(prefix) for n in names)
        assert not any('..' in Path(n).parts for n in names)
        assert not any(n.endswith(('.o', '.a', '.dSYM')) or '/.git/' in n or '/.ci/' in n for n in names)
        assert all('/Contents/MacOS/' not in n and '.sdk/' not in n for n in names)
        info = plistlib.loads(package.read(prefix + 'Info.plist'))
        assert info['CFBundleIdentifier'] == 'chaoge.ChargeLimiter'
        assert info['CFBundleExecutable'] == 'ChargeLimiter'
        assert info['MinimumOSVersion'] == '12.0'
        assert info['CFBundleSupportedPlatforms'] == ['iPhoneOS']
        assert '$(' not in repr(info), 'Unexpanded Xcode build variables'
        for resource in ['www/index.html', 'www/js/app.js', 'www/lang.json', 'splash.png',
                         'Base.lproj/Main.storyboardc/Info.plist',
                         'Base.lproj/LaunchScreen.storyboardc/Info.plist']:
            assert prefix + resource in names, resource
        executables = {name: inspect_macho(package.read(prefix + name))
                       for name in ['ChargeLimiter', 'ChargeLimiterDaemon']}
        largest = sorted(({'path': entry.filename, 'bytes': entry.file_size}
                          for entry in package.infolist() if not entry.is_dir()),
                         key=lambda entry: entry['bytes'], reverse=True)[:10]
        return {'package': path.name, 'test_build': True, 'version': info['CFBundleShortVersionString'],
                'build': info['CFBundleVersion'], 'compressed_bytes': size,
                'uncompressed_bytes': sum(entry.file_size for entry in package.infolist()),
                'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                'executables': executables, 'largest_files': largest}


if __name__ == '__main__':
    print(json.dumps(validate(Path(sys.argv[1])), indent=2))
