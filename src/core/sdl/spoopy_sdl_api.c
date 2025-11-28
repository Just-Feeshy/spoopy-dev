#include <spoopy_api.h>
#include <spoopy_backend.h>
#include <SDL3/SDL_atomic.h>
#include <SDL3/SDL_iostream.h>

static SDL_AtomicInt should_quit;

bool spoopy_api_should_quit(void) {
	return SDL_GetAtomicInt(&should_quit);
}

void spoopy_api_request_quit(void) {
	if(SDL_CompareAndSwapAtomicInt(&should_quit, 0, 1)) {
		SPOOPY_LOG_INFO("Quit Requested");
	}
}


// Yea.. I know.. it's not exactly in any way part of the renderer API, but
// it's easier to just put it here since Kinc does have a nice abstraction for file IO
// Plus, it allows us to create our own file loading system even for other platforms later on.
// I mean, we could stretch the meaning of "Spoopy Renderer" to say it includes image loading since
// textures are a big part of rendering, which requires image loading
// Most will see through that bullshit anyway, but I don't think anyone will even care
bool spoopy_api_image_load_from_file(const char* path, spoopy_image_file_format_t file_format, spoopy_image_t* dst) {
	(void)path;
	(void)file_format;
	(void)dst;

	// For now though, we'll just have this be nothing
	return false;
}
