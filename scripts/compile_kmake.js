#!/usr/bin/env node

const { execMain } = require('./common/common');
const { execSync } = require('child_process');
const path = require('path');

function main(args) {
    const cwd = /*process.env.SPOOPY_SOURCE_ROOT ||*/ path.dirname(__filename);

    try {
        const platform = process.platform;
        const sourceDir = path.join(cwd, '..', 'src', 'renderer');
        const koreDir = path.join(cwd, '..', 'src', 'renderer', 'Kore');
        let outputDir = path.join(sourceDir, 'output');
        let kmakePath = path.join(sourceDir, 'kmake');

        let graphicsAPI;
        switch (platform) {
            case 'win32':
                graphicsAPI = 'dx12';
                break;
            case 'darwin':
                graphicsAPI = 'metal';
                break;
            case 'linux':
                graphicsAPI = 'vulkan';
                break;
            default:
                throw new Error(`Unsupported platform: ${platform}`);
        }

        // Auto-detect architecture
        let arch = 'default';
        if (process.arch === 'arm64') {
            arch = 'arm64';
        } else if (process.arch === 'x64') {
            arch = 'x64';
        }

        for (let i = 0; i < args.length; i++) {
            switch (args[i]) {
                case '-g':
                case '--graphics':
                    if (i + 1 < args.length) {
                        graphicsAPI = args[++i];
                    }
                    break;
                case '--to':
                    if (i + 1 < args.length) {
                        outputDir = args[++i];
                    }
                    break;
                case '--dev':
                    if (i + 1 < args.length) {
                        kmakePath = args[++i];
                    }
                    break;
                case '--debug':
                    console.log('Current working directory:', process.cwd());
                    console.log('Script directory:', path.dirname(__filename));
                    console.log('Arguments received:', JSON.stringify(args));
                    console.log('Process argv:', JSON.stringify(process.argv));
                    console.log('NODE_PATH:', process.env.NODE_PATH);
                    console.log('PATH:', process.env.PATH);

                    break;
            }
        }

        const cmdArgs = [
            '-g', graphicsAPI,
            '--lib',
            '--meson', // Not supported by precompiled binary
            // '--dev', kmakePath,
            '--from', sourceDir,
            '--to', outputDir
        ];

        console.log(`Running kmake with graphics API: ${graphicsAPI}`);
        console.log(`Output directory: ${outputDir}`);
        console.log(`Kmake path: ${kmakePath}`);
        console.log(`Target architecture: ${arch}`);

        let kmakeExecutable;
        if (platform === 'win32') {
            kmakeExecutable = path.join(koreDir, 'make.exe');
        } else {
            kmakeExecutable = path.join(koreDir, 'make');
        }
        console.log(`Using precompiled binary: ${kmakeExecutable}`);

        const command = `"${kmakeExecutable}" ${cmdArgs.join(' ')}`;
        const result = execSync(command, {
            cwd: sourceDir,
            encoding: 'utf8',
            stdio: ['pipe', 'pipe', 'pipe']
        });

        console.log('✅ Kmake generation successful');
        console.log(result.trim());

    } catch (error) {
        console.error('❌ Error during compilation:', error.message);
        if (error.stderr) {
            console.error('Stderr:', error.stderr.trim());
        }
        process.exit(1);
    }
}

if(require.main === module) {
    execMain(main);
}

module.exports = { main };
