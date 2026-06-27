#include "../ui_impl.h"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>
#include <renderer/spoopy_metal.h>

#include "../cimgui/imgui/imgui.h"
#include "../cimgui/imgui/backends/imgui_impl_sdl3.h"
#include "../cimgui/imgui/backends/imgui_impl_metal.mm"

extern "C" void* spoopy_api_window_get_native_handle(void);
extern "C" void* spoopy_api_window_get_native_drawable(void);

bool spoopy_ui_impl_init(void) {
	SDL_Window* window = (SDL_Window*)spoopy_api_window_get_native_handle();
	if(window == NULL) {
		return false;
	}

	if(!ImGui_ImplSDL3_InitForMetal(window)) {
		return false;
	}

	id<MTLDevice> device = spoopy_metal_init();

	if(device == nil) {
		ImGui_ImplSDL3_Shutdown();
		return false;
	}

	if(!ImGui_ImplMetal_Init(device)) {
		ImGui_ImplSDL3_Shutdown();
		return false;
	}

	return true;
}

void spoopy_ui_impl_process_event(const SDL_Event* event) {
	ImGui_ImplSDL3_ProcessEvent(event);
}

void spoopy_ui_impl_new_frame(void) {
	@autoreleasepool {
		id<CAMetalDrawable> drawable = (__bridge id<CAMetalDrawable>)spoopy_api_window_get_native_drawable();
		if(drawable == nil) {
			return;
		}

		MTLRenderPassDescriptor* render_pass_descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		render_pass_descriptor.colorAttachments[0].texture = drawable.texture;
		render_pass_descriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
		render_pass_descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

		ImGui_ImplMetal_NewFrame(render_pass_descriptor);
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
	}
}

void spoopy_ui_impl_render(void) {
	@autoreleasepool {
		id<MTLCommandBuffer> command_buffer = spoopy_metal_command_buffer();
		id<MTLRenderCommandEncoder> render_encoder = spoopy_metal_render_command_encoder();

		if(command_buffer == nil || render_encoder == nil) {
			return;
		}

		ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), command_buffer, render_encoder);
	}
}

void spoopy_ui_impl_shutdown(void) {
	ImGui_ImplMetal_Shutdown();
	ImGui_ImplSDL3_Shutdown();
}
