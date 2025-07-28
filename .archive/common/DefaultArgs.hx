package common;

class DefaultArgs {
    public var fallbackVersion: Null<String>;
    public var rootdir: String;
    public var builddir: String;
    public var buildtype: String;
    public var depfile: Null<String>;

    public function new() {
        this.fallbackVersion = null;
        this.rootdir = getProjectRoot();
        this.builddir = getBuildDir();
        this.buildtype = "auto";
        this.depfile = null;
    }

    private function getProjectRoot(): String { // Determine the project root directory
        var spoopyRoot = Sys.getEnv("SPOOPY_SOURCE_ROOT");
        if (spoopyRoot != null) {
            return Path.normalize(spoopyRoot);
        }

        var currentFile = Sys.programPath();
        var scriptDir = Path.directory(currentFile);
        var projectRoot = Path.join([scriptDir, "..", ".."]);
        return Path.normalize(FileSystem.absolutePath(projectRoot));
    }

    private function getBuildDir(): String {
        var spoopyBuild = Sys.getEnv("SPOOPY_BUILD_ROOT");
        if (spoopyBuild != null) {
            return Path.normalize(spoopyBuild);
        }
        return Path.join([this.rootdir, "build"]);
    }
}
