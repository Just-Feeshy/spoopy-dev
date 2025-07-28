package;

import common.*;

class DefaultOptions {
    public static function main():Void {
        Common.execMain(mainImpl);
    }

    private static function mainImpl(args:Array<String>):Void {
        var config = ConfigGen.get();
        var formatString = "{meson_string}";

        for(i in 0...args.length) {
            if(args[i] == "--build-type" && i+1 < args.length) {
                config.build_type = args[i + 1];
                i+=2;
            }else if(!args[i].startsWith("--")) {
                formatString = args[i];
                i++;
            }else {
                Sys.stderr().writeString("Unknown argument: " + args[i] + "\n");
                Sys.exit(1);
            }
        }

        Sys.print(config.format(formatString));
    }
}
