#include <stdlib.h>
#include <memory/spoopy_memory.h>
#include <SDL3/SDL.h>


static void SDLCALL sdlcall_free(void* ptr) {
	spoopy_static_free(ptr);
}

static void* SDLCALL sdlcall_malloc(size_t size) {
	return spoopy_static_alloc(size);
}

static void* SDLCALL sdlcall_calloc(size_t nmemb, size_t size) {
	return spoopy_static_alloc(spoopy_calc_array_size(nmemb, size));
}

static void* SDLCALL sdlcall_realloc(void* ptr, size_t size) {
	return spoopy_static_realloc(ptr, size);
}

void spoopy_memory_init_hooks(void) {
	SDL_SetMemoryFunctions(
		sdlcall_malloc,
		sdlcall_calloc,
		sdlcall_realloc,
		sdlcall_free
	);
}
