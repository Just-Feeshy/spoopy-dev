#!/usr/bin/env node

const common = require('./common/common');
const fs = require('fs');
const path = require('path');

const DEFAULT_MACOS_MIN = '13.1';
const OPUS_MACOS_MIN = '15.0';
const VALID_SUPPORT_OPUS_FILES = ['auto', 'enabled', 'disabled'];

class MacosDeploymentTargetError extends common.SpoopyError {
    constructor(message) {
        super(message);
        this.name = 'MacosDeploymentTargetError';
    }
}

function getMacosMinVersion(supportOpusFiles) {
    if (!VALID_SUPPORT_OPUS_FILES.includes(supportOpusFiles)) {
        throw new MacosDeploymentTargetError(
            `Invalid support_opus_files value: ${supportOpusFiles}. Must be one of ${VALID_SUPPORT_OPUS_FILES.join(', ')}`
        );
    }

    if (supportOpusFiles === 'disabled') {
        return DEFAULT_MACOS_MIN;
    }

    return OPUS_MACOS_MIN;
}

function getDarwinLdflagsMacos(rootdir) {
    const filename = path.join(rootdir, 'misc', 'cross', 'darwin-crappy-sdl3-hack.ini');
    const text = fs.readFileSync(filename, 'utf8');
    const match = text.match(/^ldflags_macos\s*=\s*(\[[^\n]*\])/m);

    if (match === null) {
        return '[]';
    }

    return match[1];
}

function makeNativeFile(options = {}) {
    const supportOpusFiles = options.supportOpusFiles || process.env.SPOOPY_SUPPORT_OPUS_FILES || 'auto';
    const rootdir = path.resolve(options.rootdir || common.defaultArgs.rootdir);
    const macosMin = getMacosMinVersion(supportOpusFiles);
    const ldflagsMacos = getDarwinLdflagsMacos(rootdir);

    return [
        '[constants]',
        `macos_min = '${macosMin}'`,
        `cflags = ['-mmacosx-version-min='+macos_min, '-mcpu=apple-m1']`,
        `ldflags_macos = ${ldflagsMacos}`,
        `ldflags = cflags + ldflags_macos`,
        '',
        '[properties]',
        `macos_min = '${macosMin}'`,
        '',
        '[built-in options]',
        'c_args = cflags',
        'c_link_args = ldflags',
        'objc_args = cflags',
        'objc_link_args = ldflags',
        'cpp_args = cflags',
        'cpp_link_args = ldflags',
        '',
    ].join('\n');
}

function main(args) {
    const { ArgumentParser } = require('argparse');

    const parser = new ArgumentParser({
        description: 'Generate the Darwin deployment target native-file fragment for Spoopy',
        prog: args[0],
    });

    parser.add_argument('--support-opus-files', {
        choices: VALID_SUPPORT_OPUS_FILES,
        default: process.env.SPOOPY_SUPPORT_OPUS_FILES || 'auto',
        help: 'Meson support_opus_files value used to choose the macOS deployment target',
    });

    common.addCommonArgs(parser);
    const parsedArgs = parser.parse_args(args.slice(2));
    console.log(makeNativeFile({
        rootdir: parsedArgs.rootdir,
        supportOpusFiles: parsedArgs.support_opus_files,
    }));
}

module.exports = {
    DEFAULT_MACOS_MIN,
    OPUS_MACOS_MIN,
    VALID_SUPPORT_OPUS_FILES,
    MacosDeploymentTargetError,
    getMacosMinVersion,
    getDarwinLdflagsMacos,
    makeNativeFile,
    main,
};

if (require.main === module) {
    common.execMain(main);
}
