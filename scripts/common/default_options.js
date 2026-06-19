const common = require('./common');
const { execSync } = require('child_process');
const os = require('os');
const path = require('path');

class FormatError extends common.SpoopyError {
    constructor(message) {
        super(message);
        this.name = 'FormatError';
    }
}

class ConfigValidationError extends common.SpoopyError {
    constructor(message) {
        super(message);
        this.name = 'ConfigValidationError';
    }
}

const CONFIG_FALLBACK = {
    build_type: 'auto',
    language: 'c'
};

const VALID_LANGUAGES = ['c', 'c++'];
const VALID_BUILD_TYPES = ['debug', 'release', 'auto'];
const OVERRIDE_FILE_NAME = '.BUILDCONFIG';

class Config {
    constructor(configDict = null) {
        if (configDict === null) {
            configDict = { ...CONFIG_FALLBACK };
        }

        this.build_type = configDict.build_type || 'debug';
        this.language = configDict.language || 'c';

        this.options = this._generateOptions();
        this.meson_string = this._makeMesonString();
    }

    _makeMesonString() {
        return this.options.join('\n');
    }

    _generateOptions() {
        const options = [];

        if (!VALID_LANGUAGES.includes(this.language)) {
            throw new ConfigValidationError(`Invalid language: ${this.language}. Must be one of ${VALID_LANGUAGES.join(', ')}`);
        }

        if (!['debug', 'release', 'auto'].includes(this.build_type)) {
            throw new FormatError(`Invalid build_type: ${this.build_type}. Must be 'debug', 'release', or 'auto'`);
        }

        options.push('default_library=static');

        if (this.build_type === 'debug') {
            options.push(
                'buildtype=debug',
                'strip=false',
                'b_lto=false',
                'b_ndebug=false',
                'optimization=0'
            );
        } else if (this.build_type === 'release') {
            options.push(
                'buildtype=release',
                'strip=true',
                'b_lto=false',
                'b_ndebug=if-release',
                'optimization=3'
            );
        } else if (this.build_type === 'auto') {
            if (process.env.SPOOPY_DEBUG === '1') {
                options.push(
                    'buildtype=debug',
                    'strip=false',
                    'b_lto=false',
                    'b_ndebug=false'
                );
            } else {
                options.push(
                    'buildtype=release',
                    'strip=true',
                    'b_lto=false',
                    'b_ndebug=if-release'
                );
            }
        }

        options.push(...this._getPlatformOptions());
        return options;
    }

    _getPlatformOptions() {
        const options = [];

        if (process.platform === 'win32') {
            options.push(
                'c_std=c11',
                'cpp_std=c++20',
                'cpp_winlibs=[]',
                'b_vscrt=md'
            );
        } else if (process.platform === 'darwin') {
            options.push(
                'c_std=gnu11',
                'cpp_std=gnu++20',
                'objc_std=c11',
                'c_args=-mmacosx-version-min=12.0',
                'objc_args=-mmacosx-version-min=12.0',
                'c_link_args=-mmacosx-version-min=12.0',
                'objc_link_args=-mmacosx-version-min=12.0'
            );
        } else if (process.platform.startsWith('linux')) {
            options.push(
                'c_std=gnu11',
                'cpp_std=gnu++20',
                'prefer_static=true'
            );
        }

		options.push(
			'default_library=static',
			'cglm:default_library=static',
			'opus:default_library=static',
			'opusfile:default_library=static',
			'cglm:werror=false',
			'cglm:install=false',
			'cglm:build_tests=false',
			'opus:docs=disabled',
			'opus:extra-programs=disabled',
			'opus:tests=disabled'
		);

        return options;
    }

    format(template = '{string}') {
        return template.replace(/\{(\w+)\}/g, (match, key) => {
            return this[key] !== undefined ? this[key] : match;
        });
    }
}

function get(options = {}) {
    const { rootdir = null, fallback = CONFIG_FALLBACK, args = common.defaultArgs } = options;
    const actualRootdir = rootdir !== null ? rootdir : args.rootdir;
    const configDict = { ...fallback };

    if (process.env.SPOOPY_DEFAULT_DEBUG === '1') {
        configDict.build_type = 'debug';
    }

    try {
        let resolvedRootdir = actualRootdir;
        if (resolvedRootdir === null) {
            resolvedRootdir = path.resolve(__dirname, '..', '..', '..');
        } else if (typeof resolvedRootdir === 'string') {
            resolvedRootdir = path.resolve(resolvedRootdir);
        }

        const branch = execSync('git rev-parse --abbrev-ref HEAD', {
            cwd: resolvedRootdir,
            encoding: 'utf8'
        }).trim();

        console.error(`Info: Detected git branch '${branch}', adjusting configuration`);
    } catch (error) {
        console.error("Warning: Could not determine git branch, using default configuration");
    }

    return new Config(configDict);
}

function main(args) {
    const { ArgumentParser } = require('argparse');

    const parser = new ArgumentParser({
        description: 'Generate default build configuration for Spoopy',
        prog: args[0]
    });

    parser.add_argument('format', {
        type: 'str',
        nargs: '?',
        default: '{meson_string}',
        help: 'format string; available variables: {build_type}'
    });

    parser.add_argument('--build-type', {
        choices: VALID_BUILD_TYPES,
        default: 'auto',
        help: 'Build type: debug, release, or auto (default: auto)'
    });

    parser.add_argument('--enable-tests', {
        action: 'store_true',
        help: 'Enable tests in the build configuration'
    });

    common.addCommonArgs(parser);
    const parsedArgs = parser.parse_args(args.slice(2));
    console.log(get({ args: parsedArgs }).format(parsedArgs.format));
}

module.exports = {
    FormatError,
    ConfigValidationError,
    CONFIG_FALLBACK,
    VALID_LANGUAGES,
    VALID_BUILD_TYPES,
    OVERRIDE_FILE_NAME,
    Config,
    get,
    main
};
