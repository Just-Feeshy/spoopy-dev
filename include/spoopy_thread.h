#pragma once

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

typedef enum spoopy_thread_role {
    SPOOPY_THREAD_ROLE_RENDERER = 0,
} spoopy_thread_role_t;

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
typedef unsigned long spoopy_thread_index_t;
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
    void (*safely_finalize)(void* buff);
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
    spoopy_thread_index_t chunk_thread_capacity;
    spoopy_global_thread_wrapper_t* global_threads;
} spoopy_thread_manager_t;

extern spoopy_thread_manager_t threads;
extern bool spoopy_threads_initialized;

SPOOPY_FUNC_CORE void _spoopy_internal_thread_set(spoopy_global_thread_wrapper_t* global_thread);
SPOOPY_FUNC_CORE void _spoopy_internal_thread_unset(spoopy_thread_index_t index);

#endif


#ifdef SPOOPY_ALLOW_THREAD_DESIGN

#define _spoopy_thread(t, _thrd, _atomics) \
    struct spoopy_##t##_thread { \
        spoopy_thread_index_t set_index; \
        void* data; \
        spoopy_thread_id_t id; \
        _thrd; \
        _atomics \
        char name[]; \
    }; \
    typedef struct spoopy_##t##_thread spoopy_##t##_thread_t; \
    void spoopy_##t##_thread_init(void); \
    void spoopy_##t##_thread_shutdown(void); \
    spoopy_##t##_thread_t* spoopy_##t##_thread_create(const char* name, spoopy_core_thread_data_t core_data); \
    void* spoopy_##t##_thread_wait(spoopy_##t##_thread_t* thrd); \
    bool spoopy_##t##_thread_get_result(spoopy_##t##_thread_t* thread, void** result); \
    bool spoopy_##t##_current_is_main(void);

#endif


#ifndef _spoopy_thread

#define _spoopy_thread(t, _thrd, _atomics) \
    typedef struct spoopy_##t##_thread spoopy_##t##_thread_t; \
    SPOOPY_FUNC_CORE void spoopy_##t##_thread_init(void); \
    SPOOPY_FUNC_CORE void spoopy_##t##_thread_shutdown(void); \
    SPOOPY_FUNC_CORE spoopy_##t##_thread_t* spoopy_##t##_thread_create(const char* name, spoopy_core_thread_data_t core_data); \
    SPOOPY_FUNC_CORE void* spoopy_##t##_thread_wait(spoopy_##t##_thread_t* thrd); \
    SPOOPY_FUNC_CORE bool spoopy_##t##_thread_get_result(spoopy_##t##_thread_t* thread, void** result); \
    SPOOPY_FUNC_CORE bool spoopy_##t##_current_is_main(void);

#endif // _spoopy_thread


// Maybe in the future, we can add more for kinc threads (most likely not)
// this is more less here for developers to make their own thread design
// if they want to use something else other than SDL threads, or have a different
// artchitecture for threads using SDL_Thread.

_spoopy_thread(sdl,
    SDL_Thread* thrd,
    SDL_AtomicInt ref_count;
    SDL_AtomicInt thread_state;
)

SPOOPY_FUNC_CORE void spoopy_create_core_thread_data(
    spoopy_core_thread_data_t* data,
    spoopy_thread_process_t process,
    void* args_buff,
    spoopy_thread_priority_t priority
);

SPOOPY_FUNC_CORE spoopy_thread_index_t spoopy_get_main_id(void);

#undef _spoopy_thread

#ifdef __cplusplus
}
#endif

#undef __SPOOPY_USE_CORE_THREAD_DESIGN
#undef SPOOPY_ALLOW_THREAD_DESIGN
