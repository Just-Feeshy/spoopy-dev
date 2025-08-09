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
            cwd: process.cwd(),
            stdio: 'inherit',
            shell: true
        });
    } catch (error) {
        process.exit(error.status || 1);
    }
}

if (require.main === module) {
    execMain(main);
}

module.exports = { main };
