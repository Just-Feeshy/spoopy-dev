const path = require('path');
const os = require('os');

class SpoopyError extends Error {
    constructor(message) {
        super(message);
        this.name = 'SpoopyError';
    }
}

class DefaultArgs {
    constructor() {
        this.fallbackVersion = null;
        this.rootdir = path.resolve(process.env.SPOOPY_SOURCE_ROOT || path.join(__dirname, '..', '..'));
        this.builddir = path.resolve(process.env.SPOOPY_BUILD_ROOT || path.join(this.rootdir, 'build'));
        this.buildtype = 'auto';
        this.depfile = null;
    }
}

const defaultArgs = new DefaultArgs();

function addCommonArgs(parser, options = {}) {
    const { depfile = false } = options;

    parser.add_argument('--rootdir', {
        type: 'str',
        default: defaultArgs.rootdir,
        help: `Spoopy source root directory (default: ${defaultArgs.rootdir})`
    });

    parser.add_argument('--builddir', {
        type: 'str',
        default: defaultArgs.builddir,
        help: `Spoopy build root directory (default: ${defaultArgs.builddir})`
    });

    parser.add_argument('--language', {
        type: 'str',
        default: 'c',
        help: 'Programming language to use (default: c)'
    });

    if (depfile) {
        parser.add_argument('--depfile', {
            type: 'str',
            default: defaultArgs.depfile,
            help: `Path to the dependency file (default: ${defaultArgs.depfile})`
        });
    }
}

function addCargoArgs(parser) {
    parser.add_argument('--command', {
        required: true,
        choices: ['cbuild', 'test', 'build'],
        help: 'Cargo command to execute (cbuild, test, build)'
    });

    parser.add_argument('--cargo', {
        required: true,
        type: 'str',
        help: 'Path to the Cargo executable'
    });

    parser.add_argument('--manifest-path', {
        type: 'str',
        required: true,
        help: 'Path to the Cargo manifest (Cargo.toml)'
    });

    parser.add_argument('--project-build-root', {
        type: 'str',
        required: true,
        help: 'Path to the project build root directory'
    });

    parser.add_argument('--prefix', {
        type: 'str',
        required: true,
        help: 'Prefix for the build artifacts'
    });

    parser.add_argument('--libdir', {
        type: 'str',
        required: true,
        help: 'Directory for library files'
    });

    const g = parser.add_argument_group('Optimizations');
    const group = parser.add_mutually_exclusive_group({ required: false });

    group.add_argument('--release', {
        action: 'store_true',
        help: 'Build artifacts in release mode'
    });

    group.add_argument('--optimization', {
        choices: ['0', '1', '2', '3', 's'],
        help: 'Set optimization level for the build (0, 1, 2, 3, s)',
    });
}

function execMain(func, args = null) {
    if (args === null) {
        args = process.argv;
    }

    return func(args);
}

module.exports = {
    SpoopyError,
    DefaultArgs,
    defaultArgs,
    addCommonArgs,
    addCargoArgs,
    execMain
};
