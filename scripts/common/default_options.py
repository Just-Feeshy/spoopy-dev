from . import common

import sys
import subprocess
import shlex
import os

class FormatError(common.SpoopyError):
    pass

class ConfigValidationError(common.SpoopyError):
    pass

CONFIG_FALLBACK = {
    'build_type': 'auto',
    'language': 'c', # Useful for later
}

VALID_LANGUAGES = ['c', 'c++']
VALID_BUILD_TYPES = ['debug', 'release', 'auto']
OVERRIDE_FILE_NAME = '.BUILDCONFIG'


class Config(object):

    def __init__(self, config_dict=None):
        if config_dict is None:
            config_dict = CONFIG_FALLBACK.copy()

        self.build_type = config_dict.get('build_type', 'debug')
        self.language = config_dict.get('language', 'c')

        self.options = self._generate_options()
        self.meson_string = self._make_meson_string()

    def _make_meson_string(self):
        return '\n'.join(self.options)

    def _generate_options(self):
        options = []

        if self.language not in VALID_LANGUAGES:
            raise ConfigValidationError(f"Invalid language: {self.language}. Must be one of {VALID_LANGUAGES}")

        if self.build_type not in ['debug', 'release', 'auto']:
            raise ConfigFormatError(f"Invalid build_type: {self.build_type}. Must be 'debug', 'release', or 'auto'")

        options.extend([
            'default_library=static',
        ])

        if self.build_type == 'debug':
            options.extend([
                'buildtype=debug',
                'strip=false',
                'b_lto=false',
                'b_ndebug=false',
                'optimization=0',
            ])
        elif self.build_type == 'release':
            options.extend([
                'buildtype=release',
                'strip=true',
                'b_lto=true',
                'b_ndebug=if-release',
                'optimization=3',
            ])
        elif self.build_type == 'auto':
            if os.getenv('SPOOPY_DEBUG', '0') == '1':
                options.extend([
                    'buildtype=debug',
                    'strip=false',
                    'b_lto=false',
                    'b_ndebug=false',
                ])
            else:
                options.extend([
                    'buildtype=release',
                    'strip=true',
                    'b_lto=true',
                    'b_ndebug=if-release',
                ])

        options.extend(self._get_platform_options())
        return options

    def _get_platform_options(self):
        options = []

        if sys.platform == 'win32':
            options.extend([
                'c_std=c11',
                'cpp_std=c++20',
                'cpp_winlibs=[]',
                'b_vscrt=md',
            ])
        elif sys.platform == 'darwin':
            options.extend([
                'c_std=gnu11',
                'cpp_std=gnu++20',
                'objc_std=c11',
            ])
        elif sys.platform.startswith('linux'):
            options.extend([
                'c_std=gnu11',
                'cpp_std=gnu++20',
                'prefer_static=true',
            ])

        return options

    def format(self, template='{string}'):
        return template.format(**self.__dict__)

def get(*, rootdir=None, fallback=CONFIG_FALLBACK, args=common.default_args):
    rootdir = rootdir if rootdir is not None else args.rootdir
    config_dict = fallback.copy()

    if os.getenv('SPOOPY_DEFAULT_DEBUG', '0') == '1':
        config_dict['build_type'] = 'debug'

    try:
        if rootdir is None:
            import pathlib
            rootdir = pathlib.Path(__file__).parent.parent.parent
        elif not isinstance(rootdir, os.PathLike):
            import pathlib
            rootdir = pathlib.Path(rootdir)

        branch = subprocess.check_output(
            shlex.split('git rev-parse --abbrev-ref HEAD'),
            cwd=str(rootdir),
            universal_newlines=True
        ).strip()

        print(f"Info: Detected git branch '{branch}', adjusting configuration", file=sys.stderr)
    except(subprocess.SubprocessError, OSError):
        print("Warning: Could not determine git branch, using default configuration", file=sys.stderr)

    return Config(config_dict)


def main(args):
    import argparse

    parser = argparse.ArgumentParser(
        description='Generate default build configuration for Spoopy',
        prog=args[0]
    )

    parser.add_argument('format', type=str, nargs='?', default='{meson_string}',
        help='format string; available variables: {build_type}')

    parser.add_argument('--build-type',
        choices=VALID_BUILD_TYPES,
        default='auto',
        help='Build type: debug, release, or auto (default: auto)'
    )

    parser.add_argument('--enable-tests',
        action='store_true',
        help='Enable tests in the build configuration'
    )

    common.add_common_args(parser)
    args = parser.parse_args(args[1:])
    print(get(args=args).format(template=args.format))
