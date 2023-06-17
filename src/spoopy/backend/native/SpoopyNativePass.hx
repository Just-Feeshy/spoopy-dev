package spoopy.backend.native;

import spoopy.graphics.SpoopyAccessFlagBits;
import spoopy.graphics.SpoopyPipelineStageFlagBits;

class SpoopyNativePass {
    public var handle:Dynamic;

    public function new() {
        handle = SpoopyNativeCFFI.spoopy_create_render_pass();
    }

    public function addColorAttachment(location:Int, format:Int, hasImageLayout:Bool = false) {
        SpoopyNativeCFFI.spoopy_add_color_attachment(handle, location, format, hasImageLayout);
    }

    public function addDepthAttachment(location:Int, format:Int, hasStencil:Bool = false) {
        SpoopyNativeCFFI.spoopy_add_depth_attachment(handle, location, format, hasStencil);
    }

    public function addSubpassDependency(has_external1:Bool, has_external2:Bool,
    srcStageMask:SpoopyPipelineStageFlagBits, dstStageMask:SpoopyPipelineStageFlagBits,
    srcAccessMask:SpoopyAccessFlagBits, dstAccessMask:SpoopyAccessFlagBits, dependencyFlags:Int):Void {
        SpoopyNativeCFFI.spoopy_add_subpass_dependency(handle, has_external1, has_external2, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask, dependencyFlags);
    }

    public function createSubpass():Void {
        SpoopyNativeCFFI.spoopy_create_subpass(handle);
    }

    public function createRenderpass():Void {
        SpoopyNativeCFFI.spoopy_create_renderpass(handle);
    }
}