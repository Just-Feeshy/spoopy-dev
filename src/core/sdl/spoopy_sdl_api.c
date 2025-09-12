#include <spoopy_api.h>
#include <SDL3/SDL_atomic.h>

static SDL_AtomicInt should_quit;

bool spoopy_api_should_quit(void) {
	return SDL_GetAtomicInt(&should_quit);
}

void spoopy_api_request_quit(void) {
	if(SDL_CompareAndSwapAtomicInt(&should_quit, 0, 1)) {
		SPOOPY_LOG_INFO("Quit Requested");
	}
}
