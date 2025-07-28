#!/usr/bin/env node

const { execMain } = require('./common/common');
const { execSync } = require('child_process');
const path = require('path');
const fs = require('fs');

const submoduleRegex = /^.[0-9a-f]*? ([^ ]*)(?: .*?)?/;

function handleNoGit() {
    console.warn('Git is not available or this is not a git repository');
}

function main(args) {
    const cwd = process.env.SPOOPY_SOURCE_ROOT || path.dirname(__filename);

    try {
        const result = execSync('git submodule status', {
            cwd: cwd,
            encoding: 'utf8',
            stdio: ['pipe', 'pipe', 'pipe']
        });

        const modules = result.trim().split('\n');

        for (const module of modules) {
            if (!module) continue;

            const status = module[0];

            if (status !== ' ') {
                const match = module.match(submoduleRegex);

                if (!match) {
                    console.error(`Failed to parse submodule: ${module}`);
                    continue;
                }

                const modulePath = match[1];

                if (status === '+') {
                    console.log(`Submodule \`${modulePath}\` is not in sync with HEAD. Run \`git submodule update\` if this is not intended.`);
                } else if (status === '-') {
                    console.log(`Submodule \`${modulePath}\` is not initialized. Run \`git submodule update --init\` if this is not intended.`);
                } else if (status === 'U') {
                    console.log(`Submodule \`${modulePath}\` has unresolved conflicts.`);
                } else {
                    console.log(`Status of submodule \`${modulePath}\` is unknown: ${module}`);
                }
            }
        }
    } catch (error) {
        if (error.code === 'ENOENT') {
            return handleNoGit();
        }

        if (error.stderr && error.stderr.startsWith('fatal: not a git repository')) {
            return handleNoGit();
        }

        console.error('Git error:', error.stderr ? error.stderr.trim() : error.message);
        process.exit(1);
    }
}

if (require.main === module) {
    execMain(main);
}

module.exports = { main };
