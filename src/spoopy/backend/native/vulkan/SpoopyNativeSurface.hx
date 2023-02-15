package spoopy.backend.native.vulkan;

import spoopy.backend.native.SpoopyNativeCFFI;

import lime._internal.backend.native.NativeWindow;
import lime.app.Application;

class SpoopyNativeSurface {
    public var handle:Dynamic;

    private var handle_instance:Dynamic;
    private var handle_physical:Dynamic;
    private var handle_logical:Dynamic;
    private var handle_window_surface:Dynamic;

    private var name:String;
    private var version:Array<Int>;

    public function new(window:NativeWindow, application:Application) {
        name = application.meta.get("name");
        version = recursivelyGetVersionFromString(application.meta.get("version"));

        handle_window_surface = SpoopyNativeCFFI.spoopy_create_window_surface(window.handle);
        handle_instance = SpoopyNativeCFFI.spoopy_create_instance_device(handle_window_surface, name, version[0], version[1], version[2]);
        handle_physical = SpoopyNativeCFFI.spoopy_create_physical_device(handle_instance);
        handle_logical = SpoopyNativeCFFI.spoopy_create_logical_device(handle_instance, handle_physical);

        handle = SpoopyNativeCFFI.spoopy_create_surface(handle_instance, handle_physical, handle_logical, handle_window_surface);
    }

    private function recursivelyGetVersionFromString(version:String):Array<Int> {
        var vS:Array<String> = version.split(".");
        var v:Array<Int> = [];

        for(version in vS) {
            v.push(Std.parseInt(version));
        }

        if(v.length < 3) {
            return recursivelyGetVersionFromString(version + ".0");
        }

        return v;
    }
}