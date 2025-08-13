#!/usr/bin/env node

// This is a Cargo wrapper for building and testing Rust projects.
// I wrote this ahead of time since I'm starting to use Rust more frequently.

const common = require('./common/common');
const { execMain } = require('./common/common');
const { execSync } = require('child_process');

function get(options = {}) {
    return {
        format: function(format) {
            // For now, just return empty string to prevent the error
            return '';
        }
    };
}

function main(args) {
    const { ArgumentParser } = require('argparse');

    const parser = new ArgumentParser({
        description: 'Cargo wrapper for building and testing Rust projects',
        prog: args[0]
    });

    common.addCargoArgs(parser);

    const parsed_args = parser.parse_args(args.slice(2));
    const cargo_args = ['cbuild'];

    if (parsed_args.manifest_path) {
        cargo_args.push('--manifest-path', parsed_args.manifest_path);
    }

    if (parsed_args.prefix) {
        cargo_args.push('--prefix', parsed_args.prefix);
    }

    if (parsed_args.libdir) {
        cargo_args.push('--libdir', parsed_args.libdir);
    }

    if (parsed_args.release) {
        cargo_args.push('--release');
    }

    try {
        const result = execSync(`cargo-cbuild ${cargo_args.join(' ')}`, {
            cwd: parsed_args.current_source_dir || process.cwd(),
            stdio: 'inherit',
            shell: true
        });

        if (parsed_args.extension && parsed_args.current_build_dir && parsed_args.current_source_dir) {
            const fs = require('fs');
            const path = require('path');
            const glob = require('glob');

            const buildtype = parsed_args.release ? 'release' : 'debug';

            const cargo_target_dir = path.join(parsed_args.current_source_dir, 'target');
            const pattern = path.join(cargo_target_dir, '**', buildtype, `*.${parsed_args.extension}`);
            const files = glob.sync(pattern);

            for (const file of files) {
                const dest = path.join(parsed_args.current_build_dir, path.basename(file));
                fs.copyFileSync(file, dest);
                console.log(`Copied ${file} to ${dest}`);
            }

            if (files.length === 0) {
                console.warn(`Warning: No .${parsed_args.extension} files found in ${pattern}`);
            }
        }
    } catch (error) {
        process.exit(error.status || 1);
    }
}

if (require.main === module) {
    execMain(main);
}

module.exports = { main };
