package spoopy.graphics;

#if (spoopy_vulkan || spoopy_metal)
import spoopy.graphics.other.SpoopySwapChain;
#end

import lime.utils.BytePointer;
import lime.utils.ArrayBufferView;

#if (spoopy_vulkan || spoopy_metal)
@:access(spoopy.graphics.other.SpoopySwapChain)
#end
class SpoopyBuffer {
    public var data(default, null):ArrayBufferView;
    public var length(default, null):Int;
    public var pool(default, null):UInt;

    @:noCompletion var __cachedBackend:Array<SpoopyBufferBackend>;
    @:noCompletion var __backend:SpoopyBufferBackend;

    #if (spoopy_vulkan || spoopy_metal)
    @:noCompletion var __device:SpoopySwapChain;
    #end

    public function new(data:ArrayBufferView, length:Int, pool:UInt = 256) {
        this.data = data;
        this.length = length;
        this.pool = pool;

        __cachedBackend = [];
    }

    public function updateBuffers(data:ArrayBufferView, length:Int):Void {
        #if (spoopy_vulkan || spoopy_metal)
        if(__device == null) {
            return;
        }
        #end

        if(__backend.data == data && __backend.length == length && __backend == null) {
            return;
        }

        var index:Int = 0;
        var bb:SpoopyBufferBackend = null;

        while(index < __cachedBackend.length) {
            bb = __cachedBackend[index++];

            if(Reflect.compare(bb.data, data) == 0 && bb.length == length) {
                __backend = bb;
                return;
            }
        }

        __cachedBackend.insert(createBackendBuffer(data, length));
        __backend = __cachedBackend[0];

        if(__cachedBackend.length > pool) {
            __cachedBackend.pop();
        }
    }

    #if (spoopy_vulkan || spoopy_metal)
    public function bindToDevice(device:SpoopySwapChain):Void {
        if(__device != null || __backend != null) {
            return;
        }

        __device = device;
        __cachedBackend.insert(createBackendBuffer(data, length));
        __backend = __cachedBackend[0];
    }
    #end

    @:noCompletion private function createBackendBuffer(data:ArrayBufferView, length:Int):SpoopyBufferBackend {
        var __bufferPointer:BytePointer = new BytePointer();
        __bufferPointer.set(data);

        return new SpoopyBufferBackend(__device.__surface, length, data);
    }
}

#if spoopy_metal
typedef SpoopyBufferBackend = spoopy.backed.native.metal.SpoopyBufferMetal;
#end