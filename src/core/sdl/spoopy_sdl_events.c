#include <spoopy_api.h>
#include <SDL3/SDL.h>


// In the future, I'm probably going to make this multithreaded
// but for now, this is fine
// Plus! I want to have threadsafety and thread management
// via a higher level languages since
// for better safety nets + easier to work with
// when it comes to threads
// Definitely not worth the effort to make this multithreaded in C

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

uint32_t sdl_first_user_event;

static bool spoopy_events_handler_quit(SDL_Event *event, void *arg);
static bool spoopy_events_handle_video(SDL_Event *event, void *arg);

static const EventHandler default_handlers[] = {
	{ .proc = spoopy_events_handler_quit, .priority = EPRIO_SYSTEM, .event_type = SDL_EVENT_QUIT, .arg = NULL },
	{ .proc = spoopy_events_handle_video, .priority = EPRIO_SYSTEM },
	{.proc = NULL, .priority = 0, .event_type = 0, .arg = NULL}
};

static EventHandler* spoopy_events_register_default_handlers(EventHandler* h);

static inline int prio_index(EventPriority prio) {
	return prio - EPRIO_FIRST;
}

EventHandler* spoopy_events_register_handlers(EventHandler* handler_ptr, uint32_t capacity, EventHandler handlers[capacity]) {
	const size_t default_count = ARRAY_SIZE(default_handlers) - 1; // Exclude null terminator
	const size_t total_count = default_count + capacity + 1; // +1 for null terminator

	handler_ptr = spoopy_heap_realloc(handler_ptr, total_count * sizeof(EventHandler));
	memcpy(handler_ptr, default_handlers, default_count * sizeof(EventHandler));

	if(handlers) {
		memcpy(handler_ptr + default_count, handlers, capacity * sizeof(EventHandler));
	}

	// Null terminate the handler array
	handler_ptr[default_count + capacity] = (EventHandler){0};

	uint32_t pcount[NUM_EPRIOS] = { 0 };

	for(EventHandler* h = handler_ptr; h->proc; ++h) {
		++pcount[prio_index(h->priority)];
	}

	for(uint32_t i=0, pos=0; i<NUM_EPRIOS; ++i) {
		uint32_t count = pcount[i];
		pcount[i] = pos;
		pos += count;
	}

	EventHandler temp[total_count];
	memcpy(temp, handler_ptr, total_count * sizeof(EventHandler));

	for(uint32_t i=0; i<total_count && temp[i].proc; ++i) {
		handler_ptr[pcount[prio_index(temp[i].priority)]++] = temp[i];
	}

	return handler_ptr;
}

void spoopy_events_init(int32_t NUM_USER_EVENTS, EventHandler** handler_ptr) {
	if(!handler_ptr) {
		SPOOPY_LOG_ERROR("handler_ptr is NULL, cannot initialize events without a proper pointer.");
		return;
	}

	if(!SDL_Init(SDL_INIT_EVENTS)) {
		SPOOPY_LOG_ERROR("SDL_Init(SDL_INIT_EVENTS) failed: %s", SDL_GetError());
	}

	sdl_first_user_event = SDL_RegisterEvents(NUM_USER_EVENTS);
	if(sdl_first_user_event == ((uint32_t)-1)) {
		SPOOPY_LOG_ERROR(
			"You have reached the maximum number of user events supported by SDL."
			"Somewhere in your code, you might of had a buffer overflow or memory corruption."
			"Or, you some how have 4294967295 user events registered, if so, congrats!"
		);
	}


	*handler_ptr = spoopy_events_register_default_handlers(*handler_ptr);
}


/* =============================================================================
 * Default Handlers
 * ============================================================================= */

static EventHandler* spoopy_events_register_default_handlers(EventHandler* h) {
	return spoopy_events_register_handlers(h, 0, NULL);
}

static bool spoopy_events_handler_quit(SDL_Event *event, void *arg) {
	(void)event;
	(void)arg;
	spoopy_api_request_quit();
	return true;
}

static bool spoopy_events_handle_video(SDL_Event *event, void *arg) {
	(void)arg;

	return false;
}


static bool spoopy_events_invoke_handler(SDL_Event *event, EventHandler *handler) {
	assert(handler->proc != NULL);

	if(!handler->event_type || (uint32_t)handler->event_type == (uint32_t)event->type) {
		return handler->proc(event, handler->arg);
	}

	return false;
}

void spoopy_events_poll(EventHandler* handlers, EventFlags flags) {
	for(;;) {
		if(!(flags & EVENT_FLAG_NOPUMP)) {
			SDL_PumpEvents();
		}

		SDL_Event events[8];
		int n_events = SDL_PeepEvents(events, ARRAY_SIZE(events), SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST);

		if(SPOOPY_UNLIKELY(n_events < 0)) {
			SPOOPY_LOG_ERROR("SDL_PeepEvents failed: %s", SDL_GetError());
		}

		if(n_events == 0) {
			break;
		}

		for(SDL_Event *e = events, *end = events + n_events; e < end; ++e) {
			for(EventHandler *h = handlers; h->proc; ++h) {
				if(spoopy_events_invoke_handler(e, h)) {
					SPOOPY_LOG_INFO("Event type=%d handled by handler", e->type);
					break;
				}
			}
		}
	}
}
