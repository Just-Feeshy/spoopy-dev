#include <spoopy_time.h>
#include <SDL3/SDL_timer.h>

const size_t SPOOPY_TIME_RESOLUTION = SDL_NS_PER_SECOND;

size_t spoopy_time_get(void) {
	return SDL_GetTicksNS();
}
