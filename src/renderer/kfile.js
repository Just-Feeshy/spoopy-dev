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
}

project.addIncludeDir('../../include');
project.addIncludeDir('Kore/backends/system/' + target_platform + '/includes/kore3/backend');

project.addFile('src/**');
project.addExclude('**.cpp');

project.flatten();
resolve(project);
