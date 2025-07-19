#define SPOOPY_ALLOW_THREAD_DESIGN
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_mutex.h>

#include <spoopy_thread.h>
#include <spoopy_log.h>
#include <memory/spoopy_memory.h>

#include "spoopy_core.h"

static_assert(SPOOPY_THREAD_PRIO_LOW == (int)SDL_THREAD_PRIORITY_LOW, "");
static_assert(SPOOPY_THREAD_PRIO_NORMAL == (int)SDL_THREAD_PRIORITY_NORMAL, "");
static_assert(SPOOPY_THREAD_PRIO_HIGH == (int)SDL_THREAD_PRIORITY_HIGH, "");
static_assert(SPOOPY_THREAD_PRIO_CRITICAL == (int)SDL_THREAD_PRIORITY_TIME_CRITICAL, "");


static void sdl_thread_try_detach(void* thrd) {
    spoopy_sdl_thread_t* raw_thrd = (spoopy_sdl_thread_t*)thrd;
    SDL_Thread* sdl_thrd = SDL_SetAtomicPointer((void**)&raw_thrd->thrd, NULL);

    if(sdl_thrd) {
        SDL_DetachThread(sdl_thrd);
    }
}

static void sdl_thread_finalize(spoopy_sdl_thread_t* thrd) {
    assert(thrd->thrd == NULL);

    _spoopy_internal_thread_unset(thrd->set_index);
    spoopy_heap_free(thrd);
}

static void sdl_thread_try_finalize(void* thread_buffer) {
    spoopy_sdl_thread_t* thrd = (spoopy_sdl_thread_t*)thread_buffer;
    if(SDL_CompareAndSwapAtomicInt(&thrd->thread_state,
                                              SPOOPY_THREAD_STATE_FINISHED,
                                              SPOOPY_THREAD_STATE_CLEANUP)) {
        sdl_thread_finalize(thrd);
    }
}

static void sdl_thread_decref_internal(spoopy_sdl_thread_t* thrd) {
    int prev_ref_count = SDL_AddAtomicInt(&thrd->ref_count, -1);
    assert(prev_ref_count > 0);

    if(prev_ref_count == 1) {
        sdl_thread_try_finalize(thrd);
    }
}

static int SDLCALL sdl_thread_entry(void* data) {
    spoopy_core_thread_data_t core_data = *(spoopy_core_thread_data_t*)data;
    spoopy_thread_id_t id = SDL_GetCurrentThreadID();

    spoopy_sdl_thread_t* thrd = (spoopy_sdl_thread_t*)core_data.buffers.thread_buffer;
    thrd->id = id;
    spoopy_global_thread_wrapper_t global_thread = {
        .id = id,
        .buffers = core_data.buffers,
        .safely_detach = sdl_thread_try_detach,
        .safely_finalize = sdl_thread_try_finalize
    };

    _spoopy_internal_thread_set(&global_thread);

    SDL_Semaphore* semaphore = (SDL_Semaphore*)core_data.buffers.semaphore_buffer;
    SDL_SignalSemaphore(semaphore);

    if(!SDL_SetCurrentThreadPriority((SDL_ThreadPriority)core_data.priority)) {
        SPOOPY_LOG_WARN(
            "Failed to set thread priority for thread '%s': %s",
            thrd->name,
            SDL_GetError()
        );
    }

    thrd->data = core_data.process(core_data.args_buff);
    bool cas_ok = SDL_CompareAndSwapAtomicInt(&thrd->thread_state,
                                              SPOOPY_THREAD_STATE_RUNNING,
                                              SPOOPY_THREAD_STATE_FINISHED);
    assert(cas_ok);

    if(SDL_GetAtomicInt(&thrd->ref_count) < 1) {
        sdl_thread_try_detach(thrd);
        sdl_thread_finalize(thrd);
    }

    return 0;
}


void spoopy_create_core_thread_data(
    spoopy_core_thread_data_t* data,
    spoopy_thread_process_t process,
    void* args_buff,
    spoopy_thread_priority_t priority
) {
    if(SPOOPY_UNLIKELY(!SDL_GetCurrentThreadID())) {
        SPOOPY_LOG_WARN("Recommend to call spoopy_create_core_thread_data() from a valid thread context, as SDL_GetCurrentThreadID() returned NULL");
    }

    assert(data != NULL);
    assert(process != NULL);

    data->process = process;
    data->args_buff = args_buff;
    data->priority = priority;
}


void spoopy_sdl_thread_init(void) {
    threads.main_thread_id = SDL_GetCurrentThreadID();
    threads.chunk_thread_capacity = ~0UL; // Empty values
    threads.global_threads = (spoopy_global_thread_wrapper_t*)calloc(
        sizeof(threads.chunk_thread_capacity) * 8,
        sizeof(spoopy_global_thread_wrapper_t)
    );

    assert_unlikely(threads.global_threads != NULL);
    assert_unlikely(threads.chunk_thread_capacity == ~0UL);
}

void spoopy_sdl_thread_shutdown(void) {

    // Get all threads that are still running and detach them
    // since free meant 1, then if we invert the capacity,
    // we get all the threads that are still running marked as 1
    spoopy_thread_index_t existing_threads = ~threads.chunk_thread_capacity;
    while(existing_threads) {
        spoopy_thread_index_t i = __builtin_ctzl(existing_threads);

        spoopy_global_thread_wrapper_t* global_thrd = &threads.global_threads[i];
        spoopy_sdl_thread_t* thrd = (spoopy_sdl_thread_t*)global_thrd->buffers.thread_buffer;
        int nref = SDL_GetAtomicInt(&thrd->ref_count);

        if(nref > 0) {
            SPOOPY_LOG_ERROR(
                "Thread '%s' still has %d references, cannot shutdown thread manager",
                thrd->name,
                nref
            );
        }

        SDL_Thread* sdl_thrd = SDL_SetAtomicPointer(
            (void**)&thrd->thrd,
            NULL
        );

        if(sdl_thrd) {
            SDL_DetachThread(sdl_thrd);
        }

        existing_threads &= existing_threads - 1; // Clear the bit at index i
    }

    spoopy_heap_free(threads.global_threads);
    threads.global_threads = NULL;
    threads.chunk_thread_capacity = ~0UL; // Reset capacity
}

spoopy_sdl_thread_t* spoopy_sdl_thread_create(
    const char* name,
    spoopy_core_thread_data_t core_data
) {
    if(SPOOPY_UNLIKELY(!SDL_GetCurrentThreadID())) {
        SPOOPY_LOG_ERROR("Failed to create thread '%s': SDL_GetCurrentThreadID() returned NULL", name);
        return NULL;
    }

    size_t name_size = strlen(name) + 1;
    spoopy_sdl_thread_t* thread = SPOOPY_FLEX_ALLOC(spoopy_sdl_thread_t, name_size, spoopy_heap);
    SDL_memcpy(thread->name, name, name_size);
    thread->ref_count.value = 1;
    thread->thread_state.value = SPOOPY_THREAD_STATE_RUNNING;

    SDL_Semaphore* smph = SDL_CreateSemaphore(0);

    if(SPOOPY_UNLIKELY(!smph)) {
        SPOOPY_LOG_ERROR("Failed to create semaphore for thread '%s': %s", name, SDL_GetError());
        goto thread_fail;
    }

    thread->thrd = SDL_CreateThread(
        sdl_thread_entry,
        name,
        &core_data
    );

    if(SPOOPY_UNLIKELY(!thread->thrd)) {
        SPOOPY_LOG_ERROR("Failed to create thread '%s': %s", name, SDL_GetError());
        goto thread_fail;
    }

    core_data.buffers.thread_buffer = thread;
    core_data.buffers.semaphore_buffer = smph;

    SDL_WaitSemaphore(smph);
    SDL_DestroySemaphore(smph);
    return thread;

thread_fail:
    SDL_DestroySemaphore(smph);
    spoopy_heap_free(thread);
    return NULL;
}

void* spoopy_sdl_thread_wait(spoopy_sdl_thread_t* thrd) {
    SDL_Thread* sdl_thrd = SDL_SetAtomicPointer((void**)&thrd->thrd, NULL);

    if(sdl_thrd) {
        SDL_WaitThread(sdl_thrd, NULL);
    }

    void* r = thrd->data;
    sdl_thread_decref_internal(thrd);
    return r;
}

bool spoopy_sdl_thread_get_result(spoopy_sdl_thread_t* thread, void** result) {
    if(SDL_GetAtomicInt(&thread->thread_state) == SPOOPY_THREAD_STATE_RUNNING) {
        return false;
    }

    if(result) {
        *result = thread->data;
    }

    return true;
}

bool spoopy_sdl_current_is_main(void) {
    return spoopy_get_main_id() == SDL_GetCurrentThreadID();
}
