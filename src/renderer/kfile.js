// Due to compiling this into a json,
// at of config will be missing what it seems as
// critical components, but it works fine since it's all
// handled via our main meson build system.

// TODO (Framework): Have kfile be generated, instead of hardcoded
// Plus, have it be regenerated or written to if characters don't match
// or something like that

const project = new Project('spoopy_renderer');
await project.addProject(findKore());

let target_platform = platform;
if (target_platform === Platform.OSX) {
    target_platform = 'macos';

    project.addFile('src/metal/**');
    project.addFile('src/kore2/macos/**');
}

project.addIncludeDir('include/kore2');
project.addIncludeDir('../../include');

project.addFile('src/kore2/**');
project.addExclude('**.cpp');

const warningFlags = [
    '-Wno-unused-variable',
    '-Wno-unused-function',
    '-Wno-unused-parameter',
    '-Wno-deprecated-declarations',
    '-Wno-sign-compare',
    '-Wno-format-security',
    '-Wno-incompatible-pointer-types',
    '-Wno-implicit-function-declaration',
    '-Wno-gnu-folding-constant',
    '-Wno-macro-redefined',
    '-Wno-unguarded-availability-new',
    '-Wno-return-mismatch',
    '-Wno-int-conversion',
    '-Wno-missing-braces',
    '-Wno-unused-but-set-variable',
    '-Wno-format',
    '-Wno-switch'
];

for (const flag of warningFlags) {
    project.addCFlag(flag);
    project.addCppFlag(flag);
}

// Define KINC_NO_MAIN to prevent the main function from being compiled
// since this is a library, not a standalone application
project.addDefine('KINC_NO_MAIN');
project.addDefine('KINC_NO_KORE');

project.flatten();
resolve(project);
