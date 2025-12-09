#include <stdlib.h>
#include <memory/spoopy_memory.h>
#include <SDL3/SDL_stdinc.h>

static void SDLCALL sdlcall_free(void* ptr) {
	spoopy_span_free(ptr);
}

static void* SDLCALL sdlcall_malloc(size_t size) {
	return spoopy_span_alloc(size);
}

static void* SDLCALL sdlcall_calloc(size_t nmemb, size_t size) {
	return spoopy_span_alloc(spoopy_calc_array_size(nmemb, size));
}
