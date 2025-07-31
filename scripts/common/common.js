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

function injectSpoopyFrameworkPath() {
    const sp = path.resolve(__dirname, '..', '..');
    const pythonPath = process.env.PYTHONPATH || '';
    const pp = pythonPath.split(path.delimiter);

    if (!pp.includes(sp)) {
        pp.unshift(sp);
        process.env.PYTHONPATH = pp.join(path.delimiter);
    }
}

function execMain(func, args = null) {
    injectSpoopyFrameworkPath();

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
    injectSpoopyFrameworkPath,
    execMain
};
