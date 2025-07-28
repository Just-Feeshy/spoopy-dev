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

    parser.option('--rootdir', {
        type: 'string',
        default: defaultArgs.rootdir,
        description: `Spoopy source root directory (default: ${defaultArgs.rootdir})`
    });

    parser.option('--builddir', {
        type: 'string', 
        default: defaultArgs.builddir,
        description: `Spoopy build root directory (default: ${defaultArgs.builddir})`
    });

    parser.option('--language', {
        type: 'string',
        default: 'c',
        description: 'Programming language to use (default: c)'
    });

    if (depfile) {
        parser.option('--depfile', {
            type: 'string',
            default: defaultArgs.depfile,
            description: `Path to the dependency file (default: ${defaultArgs.depfile})`
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
