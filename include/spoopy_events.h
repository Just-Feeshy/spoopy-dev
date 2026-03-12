#pragma once


// Full credits go towards the Taisei Team for the event system design
// Very simple, straightforward, and efficient
// Plus, very easy to expand upon

#include <spoopy.h>

typedef union SDL_Event SDL_Event;

typedef bool (*EventHandlerProc)(SDL_Event* event, void* arg);

typedef enum {
	// From top (highest priority) to bottom (lowest priority)

	EPRIO_SYSTEM = -4, // Events not associated with user input
	EPRIO_MISC,		   // Misc Events

	EPRIO_FIRST = EPRIO_SYSTEM,
	EPRIO_LAST = EPRIO_MISC,
	NUM_EPRIOS = EPRIO_LAST - EPRIO_FIRST + 1
} EventPriority;

typedef enum {
	EVENT_FLAG_NOPUMP = (1 << 0),
} EventFlags; // Built in event flags

typedef struct spoopy_event_handler {
	EventHandlerProc proc;
	void *arg;
	EventPriority priority;
	int32_t event_type; // SDL_EventType or any other type identifier
} spoopy_event_handler_t;

SPOOPY_FUNC_CORE spoopy_event_handler_t* spoopy_events_register_handlers(spoopy_event_handler_t* handler_ptr, uint32_t capacity, spoopy_event_handler_t handlers[capacity]);
SPOOPY_FUNC_CORE void spoopy_events_init(int32_t NUM_USER_EVENTS, spoopy_event_handler_t** handler_ptr);
SPOOPY_FUNC_CORE void spoopy_events_poll(spoopy_event_handler_t* handlers, EventFlags flags);
