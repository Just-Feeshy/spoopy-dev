<!-- TODO: Replace this with Zig.. that's not a joke -->
<!-- Best way to do this is by making an xml parser for the `build.zig` to use -->
<!-- But there's the problem that developers will have to compile for more than one language -->

<xml>
	<set name="HXCPP_CPP11" value="1" />
	<set name="PLATFORM" value="android-21" if="android" />

	<set name="ios" value="1" if="iphone" />
	<set name="tvos" value="1" if="appletv" />
	<set name="mac" value="1" if="macos" />

	<set name="SPOOPY_HASHLINK" value="1" if="hashlink" />
	<set name="SPOOPY_SDL" value="1" />
	<set name="SPOOPY_ROCKET" value="1" />
	<set name="SPOOPY_SDL_SUPPORT_RENDERER" value="1" />
	<set name="SPOOPY_SDL_ANGLE" value="1" if="windows angle" unless="static_link" />
	<set name="SPOOPY_SDL_ANGLE" value="1" if="winrt" />
	<set name="NATIVE_TOOLKIT_PATH" value="../lime-project/lib" unless="NATIVE_TOOLKIT_PATH" />

	<set name="NATIVE_TOOLKIT_SDL_STATIC" value="1" />
	<set name="NATIVE_TOOLKIT_SDL_ANGLE" value="1" if="SPOOPY_SDL_ANGLE" />

	<set name="INCLUDE_SDL" value="1" if="SPOOPY_SDL" />
	<set name="USE_CUSTOM_SDL_DEFINITIONS" value="1" if="SPOOPY_SDL" />

	<set name="LIME_SOURCE_PATH" value="../lime-project/src" />
	<set name="LIME_INCLUDE_PATH" value="../lime-project/include" />
	<!--<set name="LIME_OPENGL_FLAG" value="1"/>-->

	<set name="LIME_METAL" value="1" if="SPOOPY_METAL" />
	<set name="LIME_ENABLE_GL_CONTEXT" value="1" unless="SPOOPY_METAL || SPOOPY_VULKAN" />
	<set name="LIME_OPENGL" value="1" unless="SPOOPY_METAL || SPOOPY_VULKAN" />
	<set name="SPOOPY_SPIRV_CROSS" value="1" unless="SPOOPY_OPENGL || SPOOPY_VULKAN" />

	<section if="mac">
		<setenv name="MACOSX_DEPLOYMENT_TARGET" value="10.9" if="HXCPP_CPP11 || HXCPP_CPP14" />
		<setenv name="MACOSX_DEPLOYMENT_TARGET" value="10.7" if="OBJC_ARC" unless="MACOSX_DEPLOYMENT_TARGET" />
		<setenv name="MACOSX_DEPLOYMENT_TARGET" value="10.6" unless="MACOSX_DEPLOYMENT_TARGET" />
	</section>

	<!-- Arguements (Meaing that they ARE included, but only through arguments) -->
	<!--<set name="SPOOPY_INCLUDE_EXAMPLE" value="1" />-->
	<!--<set name="SPOOPY_VULKAN" value="1" />-->
	<!--<set name="SPOOPY_METAL" value="1" />-->

	<set name="SPOOPY_VOLK" value="1" if="SPOOPY_VULKAN" />

	<section unless="OUTPUT_DIR">
		<set name="OUTPUT_DIR" value="../ndll-metal" if="SPOOPY_METAL" />
		<set name="OUTPUT_DIR" value="../ndll-vulkan" if="SPOOPY_VULKAN" />
		<set name="OUTPUT_DIR" value="../ndll" unless="SPOOPY_VULKAN || SPOOPY_METAL" />
	</section>

	<files id="lime-extra">
        <!-- EMPTY -->
	</files>

	<files id="lime">
		<compilerflag value="-DLIME_DEBUG" if="SPOOPY_DEBUG" />
		<compilerflag value="-DLIME_ENABLE_GL_CONTEXT" if="LIME_ENABLE_GL_CONTEXT" />
		<compilerflag value="-DLIME_OPENGL_FLAG" if="LIME_OPENGL_FLAG" />
		<compilerflag value="-DLIME_METAL" if="LIME_METAL" />

		<compilerflag value="-DLIME_OPENGL" if="LIME_OPENGL" />

		<section if="SPOOPY_VULKAN">
			<compilerflag value="-DVK_USE_IMAGE_ACQUIRE_FENCES=1" />
			<compilerflag value="-DVK_MAX_BACK_BUFFERS=4" />
			<compilerflag value="-DVK_BACK_BUFFERS_COUNT=3" />

			<compilerflag value="-Ithirdparty/vk" />
			<compilerflag value="-IVulkanSDK/include" />
		</section>

		<section if="SPOOPY_DEBUG">
            		<linkerflag value="-fsanitize=thread" />
		</section>

		<section if="SPOOPY_SPIRV_CROSS">
			<compilerflag value="-DSPIRV_ONLY" />
			<compilerflag value="-DSPOOPY_SPIRV_CROSS" />
			<compilerflag value="-Ilib/spirv-cross/include/spirv_cross/" />
			<compilerflag value="-Ilib/spirv-cross/" />
		</section>

		<section if="SPOOPY_ROCKET">
		    <compilerflag value="-DSPOOPY_ROCKET" />
		    <compilerflag value="-Ilib/rocket/" />
		</section>

		<section if="SPOOPY_VOLK">
			<compilerflag value="-DSPOOPY_VOLK" />
			<!--<compilerflag value="-DLIME_VOLK" />-->
			<!--<compilerflag value="-DVK_NO_PROTOTYPES" />-->
			<!--<compilerflag value="-DVOLK_IMPLEMENTATION" />-->
			<compilerflag value="-Ilib/volk/" />
		</section>

		<section if="SPOOPY_DEBUG">
		    <compilerflag value="-DSPOOPY_DEBUG" />
		</section>

		<file name="src/SpoopyExternalInterface.cpp" />

		<compilerflag value="-I${LIME_INCLUDE_PATH}" />
		<compilerflag value="-I${LIME_SOURCE_PATH}/backend/sdl" if="SPOOPY_SDL" />
		<compilerflag value="-Ithirdparty" />

		<compilerflag value="-DSPOOPY_SDL" if="SPOOPY_SDL" />
		<compilerflag value="-DSPOOPY_VULKAN" if="SPOOPY_VULKAN" />
		<compilerflag value="-DLIME_VULKAN" if="SPOOPY_VULKAN" />
		<compilerflag value="-DSPOOPY_INCLUDE_EXAMPLE" if="SPOOPY_INCLUDE_EXAMPLE" />
		<compilerflag value="-DUSE_CUSTOM_SDL_DEFINITIONS" if="USE_CUSTOM_SDL_DEFINITIONS"/>

		<section if="SPOOPY_VULKAN">
			<file name="thirdparty/vk/vk_mem_alloc.cpp" />
			<file name="src/graphics/vulkan/shaders/PipelineShader.cpp" />
			<file name="src/graphics/vulkan/shaders/Shader.cpp" />
			<!--<file name="src/graphics/vulkan/images/Image.cpp" />-->
			<file name="src/graphics/vulkan/PipelineVulkan.cpp" />
			<file name="src/graphics/vulkan/ContextVulkan.cpp" />
			<file name="src/graphics/vulkan/ContextStage.cpp" />
			<file name="src/graphics/vulkan/GraphicsHandlerVulkan.cpp" />
			<file name="src/graphics/vulkan/GraphicsVulkan.cpp" />
			<file name="src/graphics/vulkan/SwapchainVulkan.cpp" />
			<file name="src/graphics/vulkan/components/CommandPoolVulkan.cpp" />
			<file name="src/graphics/vulkan/components/SemaphoreVulkan.cpp" />
			<file name="src/graphics/vulkan/components/FenceVulkan.cpp" />
			<file name="src/graphics/vulkan/RenderPassVulkan.cpp" />
			<file name="src/graphics/vulkan/FramebufferVulkan.cpp" />
			<file name="src/graphics/vulkan/QueueVulkan.cpp" />
			<file name="src/graphics/vulkan/CommandBufferVulkan.cpp" />
			<file name="src/graphics/vulkan/EntryVulkan.cpp" />
			<file name="src/helpers/SpoopyHelpersVulkan.cpp" />
			<file name="src/helpers/VulkanAddons.cpp" />
			<file name="src/device/Capabilities.cpp" />
			<file name="src/device/Instance.cpp" />
			<file name="src/device/PhysicalDevice.cpp" />
			<file name="src/device/LogicalDevice.cpp" />
			<file name="src/device/Surface.cpp" />

			<file name="src/device/MacVulkanBindings.mm" if="mac" />

		</section>

        <file name="src/utils/MemoryReader.cpp" />
        <file name="src/utils/Time.cpp" />
        <file name="src/core/Engine.cpp" />
        <file name="src/cffi/UnsafeBindings.cpp" />
        <file name="src/graphics/EmptyOpenGL.cpp" unless="LIME_OPENGL"/>
        <file name="src/graphics/Viewport.cpp" />
        <file name="src/system/sdl/SDLScopeLock.cpp" />
        <file name="src/system/sdl/SDLSemaphore.cpp" />
	</files>

	<include name="volk.xml.tpl" if="SPOOPY_VOLK" />
	<include name="spirv-cross.xml.tpl" if="SPOOPY_SPIRV_CROSS" />
	<include name="rocket.xml.tpl" if="SPOOPY_ROCKET" />

	<target id="lime" output="${LIBPREFIX}lime${DEBUGEXTRA}${LIBSUFFIX}" tool="linker" toolid="${STD_MODULE_LINK}">
		<files id="spoopy-toolkit-volk" if="SPOOPY_VOLK" />
		<files id="spoopy-toolkit-spirv" if="SPOOPY_SPIRV_CROSS" />

        <cppflag value="-std=c++11" if="HXCPP_CPP11" />

        <flag value="-g" if="SPOOPY_DEBUG"/>

		<section unless="static_link">
			<section if="mac">
				<vflag name="-framework" value="Metal" />
				<vflag name="-framework" value="MetalKit" />
				<vflag name="-framework" value="QuartzCore" />
				<vflag name="-framework" value="IOKit" />
				<vflag name="-framework" value="IOSurface" />

				<!--<vflag name="-arch" value="arm64" />-->
			</section>

			<section if="ios">
				<vflag name="-framework" value="Metal" />
				<vflag name="-framework" value="MetalKit" />
				<vflag name="-framework" value="QuartzCore" />
				<vflag name="-framework" value="UIKit" />
				<vflag name="-framework" value="IOSurface" />
			</section>
		</section>
	</target>

	<include name="../lime-project/Build.xml" />
</xml>
