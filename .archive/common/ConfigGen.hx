package common;

import sys.io.Process;

class ConfigGen {
    public static function get(): Config {
        var buildType = "auto";

        if(Sys.getEnv("SPOOPY_DEFAULT_DEBUG") == "1") {
            buildType = "debug";
        }

        try {
            final process = new Process("git", ["rev-parse", "--abbrev-ref", "HEAD"]);
            final _out = process.stdout.readAll();
            final _in = process.stderr.readAll();

            final stdoutContent = _out.toString();
            final stderrContent = _in.toString();

            final exitCode = process.exitCode();
            process.close();

            if(exitCode != 0) {
                onProcessFail(process, exitCode, stdoutContent, stderrContent);
                return null;
            }else {
                if(stdoutContent.length > 0) {
                    Sys.println("Current branch: " + stdoutContent.trim());
                }

                if(stderrContent.length > 0) {
                    Sys.println(stderrContent);
                }
            }
        }catch(e:haxe.Exception) {
            Sys.stderr().writeString("Warning: Could not determine git branch, using default configuration\n");
        }

        return new Config(buildType);
    }

    private static function onProcessFail(
        process:sys.io.Process,
        ec:Int,
        stdoutContent:String,
        stderrContent:String
    ):Void {
        final info = [];
        info.push("Exit Code:\n" + ec);

        if(stdoutContent.length > 0) {
            info.push("Stdout:\n" + stdoutContent);
        }

        if(stderrContent.length > 0) {
            info.push("Stderr:\n" + stderrContent);
        }

        var result = "\nFAILURE INFO\n------------------------------------\n";
        result += info.join("\n");
        result += "\n------------------------------------\n";

        printFailed(result);
    }

    private static function printFailed(msg:Null<String> = null) {
        printlnErr("Failed 💔 when generating config.\n" +
            "Please check the error messages below for more details.");

        if(msg != null) {
            printlnErr(msg);
        }
    }

    private static function printlnErr(msg:String) {
        Sys.stderr().writeString(msg + "\n", haxe.io.Encoding.UTF8);
        Sys.stderr().flush();
    }
}
