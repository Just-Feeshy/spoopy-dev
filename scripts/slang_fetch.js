#!/usr/bin/env node

const { execMain } = require('./common/common');
const { execSync } = require('child_process');
const os = require('os');
const fs = require('fs');
const path = require('path');

function detect_platform() {
    const platform = process.platform;
    const arch = process.arch;

    const osName = {
        'win32': 'windows',
        'darwin': 'macos',
        'linux': 'linux'
    }[platform] || platform;

    const archName = {
        'x64': 'x86_64',
        'ia32': 'x86',
        'arm64': 'aarch64'
    }[arch] || arch;

    return { osName, arch: archName };
}

async function download_slang(version, output_dir) {
    const { osName: os_name, arch } = detect_platform();
    const filename = `slang-${version}-${os_name}-${arch}.tar.gz`;
    const url = `https://github.com/shader-slang/slang/releases/download/v${version}/${filename}`;

    console.log(`Downloading Slang version ${version} for ${os_name}-${arch}...`);
    console.log(`URL: ${url}`);

    const output_path = path.resolve(output_dir);
    fs.mkdirSync(output_path, { recursive: true });

    const zip_path = path.join(output_path, filename);
    console.log(`Downloading to ${zip_path}...`);

    const response = await fetch(url);
    if (!response.ok) {
        throw new Error(`Failed to download Slang: ${response.statusText}`);
    }

    const buffer = await response.arrayBuffer();
    fs.writeFileSync(zip_path, Buffer.from(buffer));
    console.log(`Downloaded Slang to ${zip_path}`);

    console.log(`Extracting Slang archive...`);
    
    // Extract tar.gz file using system tar command
    execSync(`tar -xzf "${zip_path}" -C "${output_path}"`, { stdio: 'inherit' });
    console.log(`Slang extracted successfully`);

    fs.unlinkSync(zip_path);
    console.log(`Removed downloaded archive: ${zip_path}`);

    // The tarball extracts directly into the output directory
    const extract_path = output_path;

    let lib_path, dll_path, exe_path;

    if (os_name === 'windows') {
        lib_path = path.join(extract_path, 'lib', 'slang.lib');
        dll_path = path.join(extract_path, 'bin', 'slang.dll');
        exe_path = path.join(extract_path, 'bin', 'slang.exe');
    } else {
        if (os_name === 'linux') {
            lib_path = path.join(extract_path, 'lib', 'libslang.so');
        } else {
            lib_path = path.join(extract_path, 'lib', 'libslang.dylib');
        }

        dll_path = lib_path;
        exe_path = path.join(extract_path, 'bin', 'slangc');
    }

    const include_path = path.join(extract_path, 'include');

    return {
        root: extract_path,
        include: include_path,
        lib: lib_path,
        dll: dll_path,
        exe: exe_path
    };
}

async function main(args) {
    if (args.length !== 4) {
        console.error("Usage: slang_fetch <version> [output_dir]");
        process.exit(1);
    }

    const version = args[2];
    const output_dir = args[3];

    try {
        const paths = await download_slang(version, output_dir);

        console.log('Successfully downloaded and extracted Slang:');
        console.log('Paths:', paths);

        const paths_file = path.join(output_dir, 'slang_paths.txt');
        const paths_content = Object.entries(paths)
            .map(([key, value]) => `${key}=${value}`).join('\n');

        fs.writeFileSync(paths_file, paths_content);
        console.log(`Slang paths saved to ${paths_file}`);
    } catch (error) {
        console.error("Error downloading Slang:", error.message);
        process.exit(1);
    }
}

if (require.main === module) {
    execMain(main);
}

module.exports = { main };
