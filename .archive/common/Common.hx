package common;

class Common {
    public static function execMain(func:Array<String>->Void, ?args:Array<String>):Void {
        if (args == null) {
            args = Sys.args();
        }

        try {
            func(args);
        } catch (e: haxe.Exception) {
            Sys.stderr().writeString('Unexpected Error: ${e.message}\n');
            Sys.exit(1);
        }
    }
}
