#include "../ui_impl.h"

#include <renderer/spoopy_metal.h>

#include "../cimgui/imgui/backends/imgui_impl_metal.mm"

bool spoopy_ui_impl_init(void) {
	id<MTLDevice> device = spoopy_metal_init();

	if(device == nil) {
		return false;
	}

	return ImGui_ImplMetal_Init(device);
}

void spoopy_ui_impl_shutdown(void) {
	ImGui_ImplMetal_Shutdown();
}
