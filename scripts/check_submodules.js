#!/usr/bin/env node

const { execMain } = require('./common/common');
const { execSync } = require('child_process');
const path = require('path');

const submoduleRegex = /^.[0-9a-f]*? ([^ ]*)(?: .*?)?/;

function handleNoGit() {
    console.error('Git is not available or this is not a git repository');
}

function initializeSubmodule(modulePath, cwd) {
    try {
        console.error(`Initializing submodule: ${modulePath}`);
        execSync(`git submodule update --init "${modulePath}"`, {
            cwd: cwd,
            encoding: 'utf8',
            stdio: ['pipe', 'pipe', 'pipe']
        });
        console.error(`✅ Successfully initialized submodule: ${modulePath}`);
        return true;
    } catch (error) {
        console.error(`❌ Failed to initialize submodule ${modulePath}:`, error.stderr ? error.stderr.trim() : error.message);
        return false;
    }
}

function main(args) {

    const cwd = /*process.env.SPOOPY_SOURCE_ROOT ||*/ path.dirname(__filename);
    const uninitializedModules = [];

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
            if (status === '-') {  // ONLY check for uninitialized
                const match = module.match(submoduleRegex);
                if (!match) {
                    console.error(`Failed to parse submodule: ${module}`);
                    continue;
                }

                const modulePath = match[1];
                uninitializedModules.push(modulePath);
            }
        }

        if (uninitializedModules.length > 0) {

            uninitializedModules.forEach(path => {
                console.log(`Submodule '${path}' was uninitialized`);
            });

            const failedInits = [];
            for (const modulePath of uninitializedModules) {
                const success = initializeSubmodule(modulePath, cwd);
                if (!success) {
                    failedInits.push(modulePath);
                }
            }

            if (failedInits.length > 0) {
                console.error(`Failed to initialize ${failedInits.length} submodule(s)`);
                process.exit(1);
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
