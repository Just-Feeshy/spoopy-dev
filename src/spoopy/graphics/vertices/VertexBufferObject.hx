package spoopy.graphics.vertices;

import spoopy.graphics.SpoopyBuffer;
import spoopy.util.SpoopyFloatBuffer;

#if (spoopy_vulkan || spoopy_metal)
import spoopy.graphics.other.SpoopySwapChain;
#end

/*
* Base helper class for managing buffers.
*/
class VertexBufferObject {
    public var buffers(default, null):Array<SpoopyFloatBuffer>;

    public var offset(default, null):Int;
    public var length:Int;

    @:noCompletion var __vertices:SpoopyBuffer;

    #if (spoopy_vulkan || spoopy_metal)
    @:noCompletion var __device:SpoopySwapChain;
    #end

    public function new() {
        __buffers = [];

        offset = 0;
        length = 0;
    }

    #if (spoopy_vulkan || spoopy_metal)
    public function bindToDevice(device:SpoopySwapChain):Void {
        __device = device;
    }
    #end

    public function create():Void {
        if(__vertices == null) {
            __vertices = new SpoopyBuffer(outputBuffers, outputBuffers.byteLength);
            __vertices.init();

            return;
        }

        __vertices.updateBuffers(outputBuffers, outputBuffers.byteLength);
    }

    public function update():Void {
        var outputBuffers:SpoopyFloatBuffer = new SpoopyFloatBuffer(length);
        var __length = 0;

        for(i in 0...buffers.length) {
            var b = buffers[i];

            outputBuffers.set(b, __length);
            __length += b.length;
        }
    }
}