#ifndef SPOOPY_THREAD_H
#define SPOOPY_THREAD_H

#include <spoopy.h>


/* ==================================================================================
 * DO NOT DEFINE `__SPOOPY_USE_CORE_THREAD_DESIGN` UNLESS YOU KNOW WHAT YOU ARE DOING
 * ================================================================================== */

#ifdef SPOOPY_ALLOW_THREAD_DESIGN
#define __SPOOPY_USE_CORE_THREAD_DESIGN
#endif // __SPOOPY_ALLOW_THREAD_DESIGN

#ifdef __cplusplus
extern "C" {
#endif

typedef enum spoopy_thread_state {
    SPOOPY_THREAD_STATE_RUNNING,
    SPOOPY_THREAD_STATE_FINISHED,
    SPOOPY_THREAD_STATE_CLEANUP
} spoopy_thread_state_t;

typedef enum spoopy_thread_priority {
    SPOOPY_THREAD_PRIO_LOW,
    SPOOPY_THREAD_PRIO_NORMAL,
    SPOOPY_THREAD_PRIO_HIGH,
    SPOOPY_THREAD_PRIO_CRITICAL
} spoopy_thread_priority_t;

typedef uint64_t spoopy_thread_id_t;
typedef void *(*spoopy_thread_process_t)(void* arg);

typedef struct spoopy_thread_buffers {
    void* thread_buffer;
    void* semaphore_buffer;
} spoopy_thread_buffers_t;

typedef struct {
    spoopy_thread_buffers_t buffers;
    spoopy_thread_process_t process;
    spoopy_thread_priority_t priority;
    void* args_buff;
} spoopy_core_thread_data_t;


#ifdef __SPOOPY_USE_CORE_THREAD_DESIGN

typedef struct spoopy_global_thread_wrapper {
    spoopy_thread_id_t id;
    spoopy_thread_buffers_t buffers;
    void (*safely_detach)(void* buff);
    void (*safely_finalize)(struct spoopy_global_thread_wrapper* buff);
} spoopy_global_thread_wrapper_t;



// Maximum number of threads that can be created is 64 only or 32 for lower-end systems.
// This is a design choice to keep the thread management simple and efficient.
//
// So for our `chunk_thread_capacity`, if our bit is 1,
// it means the thread is active (meaning empty), if it's 0,
// it means the thread is inactive.
// It's faster to check free threads this way.
//
// Copy this to every source file for thread design.

typedef struct {
    spoopy_thread_id_t main_thread_id;
    unsigned long chunk_thread_capacity;
    spoopy_global_thread_wrapper_t* global_threads;
} spoopy_thread_manager_t;

extern spoopy_thread_manager_t threads;

#endif


#ifdef SPOOPY_ALLOW_THREAD_DESIGN

#define _spoopy_thread(t, _thrd, _atomics) \
    struct spoopy_##t##_thread { \
        void* data; \
        spoopy_thread_id_t id; \
        _thrd; \
        _atomics \
        char name[]; \
    }; \
    typedef struct spoopy_##t##_thread spoopy_##t##_thread_t; \
    void spoopy_##t##_thread_init(void); \
    spoopy_##t##_thread_t* spoopy_##t##_thread_create(const char* name, spoopy_core_thread_data_t core_data);

#endif


#ifndef _spoopy_thread

#define _spoopy_thread(t, _thrd, _atomics) \
    typedef struct spoopy_##t##_thread spoopy_##t##_thread_t; \
    SPOOPY_FUNC_CORE void spoopy_##t##_thread_init(void); \
    SPOOPY_FUNC_CORE spoopy_##t##_thread_t* spoopy_##t##_thread_create(const char* name, spoopy_core_thread_data_t core_data);

#endif // _spoopy_thread


#if SPOOPY_SUPPORT_SDL_THREADS

_spoopy_thread(sdl,
    SDL_Thread* thrd,
    SDL_AtomicInt ref_count;
    SDL_AtomicInt thread_state;
)

#endif // SPOOPY_SUPPORT_SDL_THREADS

SPOOPY_FUNC_CORE void _spoopy_internal_thread_set(void* thread_buffer);
SPOOPY_FUNC_CORE void _spoopy_internal_thread_unset(void* thread_buffer);

SPOOPY_FUNC_CORE void spoopy_create_core_thread_data(
    spoopy_core_thread_data_t* data,
    spoopy_thread_process_t process,
    void* args_buff,
    spoopy_thread_priority_t priority
);

#ifdef __cplusplus
}
#endif


#undef _spoopy_thread

#ifdef __SPOOPY_USE_CORE_THREAD_DESIGN
#undef __SPOOPY_USE_CORE_THREAD_DESIGN
#endif // __SPOOPY_USE_CORE_THREAD_DESIGN

#ifdef SPOOPY_ALLOW_THREAD_DESIGN
#undef SPOOPY_ALLOW_THREAD_DESIGN
#endif // SPOOPY_ALLOW_THREAD_DESIGN

#endif
