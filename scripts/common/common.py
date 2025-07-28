from pathlib import Path

import os

class SpoopyError(RuntimeError):
    pass

class DefaultArgs(object):
    def __init__(self):
        self.fallback_version = None
        self.rootdir = Path(os.environ.get('SPOOPY_SOURCE_ROOT', Path(__file__).parent.parent)).resolve()
        self.builddir = Path(os.environ.get('SPOOPY_BUILD_ROOT', self.rootdir / 'build')).resolve()
        self.buildtype = 'auto'
        self.depfile = None

default_args = DefaultArgs()


def add_common_args(parser, *, depfile=False):
    parser.add_argument('--rootdir',
        type=Path,
        default=default_args.rootdir,
        help='Spoopy source root directory (default: %(default)s)'
    )

    parser.add_argument('--builddir',
        type=Path,
        default=default_args.builddir,
        help='Spoopy build root directory (default: %(default)s)'
    )

    parser.add_argument('--language',
        type=str,
        default='c',
        help='Programming language to use (default: c)'
    )

    if depfile:
        parser.add_argument('--depfile',
            type=Path,
            default=default_args.depfile,
            help='Path to the dependency file (default: %(default)s)'
        )

def inject_spoopy_framework_path():
    sp = str(Path(__file__).parent.parent.resolve())
    pp = os.environ.get('PYTHONPATH', '').split(os.pathsep)

    if sp not in pp:
        pp.insert(0, sp)
        os.environ['PYTHONPATH'] = os.pathsep.join(pp)

def exec_main(func, args=None):
    inject_spoopy_framework_path();

    if args is None:
        import sys
        args = sys.argv

    return func(args)
