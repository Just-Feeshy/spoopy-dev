package common;

class Config {
    public var build_type: String;
    public var options: Array<String>;
    public var meson_string: String;

    @:allow(common.ConfigGen) private function new(?buildType: String) {
        this.build_type = buildType != null ? buildType : "auto";

        final validBuildTypes = ["debug", "release", "auto"];
        if(validBuildTypes.indexOf(this.build_type) == -1) {
            throw new haxe.Exception('Invalid build type: ${this.build_type}$. Must be debug, release, or auto');
        }

        this.options = generateOptions();
        this.meson_string = makeMesonString();
    }

    private function generateOptions():Array<String> {
        var options = [];

        options.push("c_std=c99");
        options.push("default_library=static");

        if (this.build_type == "debug") {
            options.push("buildtype=debug");
            options.push("strip=false");
            options.push("b_lto=false");
            options.push("b_ndebug=false");
            options.push("optimization=0");
        } else if (this.build_type == "release") {
            options.push("buildtype=release");
            options.push("strip=true");
            options.push("b_lto=true");
            options.push("b_ndebug=if-release");
            options.push("optimization=3");
        } else if (this.build_type == "auto") {
            if (Sys.getEnv("SPOOPY_DEBUG") == "1") {
                options.push("buildtype=debug");
                options.push("strip=false");
                options.push("b_lto=false");
                options.push("b_ndebug=false");
            } else {
                options.push("buildtype=release");
                options.push("strip=true");
                options.push("b_lto=true");
                options.push("b_ndebug=if-release");
            }
        }

        var platformOptions = getPlatformOptions();
        for (option in platformOptions) {
            options.push(option);
        }

        return options;
    }

    private function getPlatformOptions(): Array<String> {
        var options = [];
        var systemName = Sys.systemName();

        if (systemName == "Windows") {
            options.push("cpp_winlibs=[]");
            options.push("b_vscrt=md");
        } else if (systemName == "Mac") {
            options.push("objc_std=c11");
        } else if (systemName == "Linux") {
            options.push("prefer_static=true");
        }

        return options;
    }

    private function makeMesonString():String {
        return this.options.join("\n");
    }

    public function format(template: String):String {
        var result = template;
        result = StringTools.replace(result, "{build_type}", this.build_type);
        result = StringTools.replace(result, "{meson_string}", this.meson_string);
        return result;
    }
}
