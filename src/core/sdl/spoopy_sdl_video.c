#include <spoopy_video.h>
#include <spoopy_log.h>
#include <SDL3/SDL.h>

void __spoopy_video_internal_init_backend(void) {
	SDL_SetHintWithPriority(SDL_HINT_NO_SIGNAL_HANDLERS,"1",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH,"0",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE,"1",SDL_HINT_OVERRIDE);
    SPOOPY_LOG_INFO("SDL hints set - QUIT_ON_LAST_WINDOW_CLOSE=1");

#if defined(__APPLE__)
    SDL_SetHintWithPriority(SDL_HINT_MAC_BACKGROUND_APP,"0",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES,"0",SDL_HINT_OVERRIDE);
#endif
}
