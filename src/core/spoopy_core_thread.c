#define __SPOOPY_USE_CORE_THREAD_DESIGN
#include "spoopy_core.h"

#if SPOOPY_SUPPORT_SDL_THREADS

#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_atomic.h>

#endif

#include <spoopy_thread.h>

spoopy_thread_manager_t threads = {0};
bool spoopy_threads_initialized = false;

void _spoopy_internal_thread_set(spoopy_global_thread_wrapper_t* thread_buffer) {
    if(SPOOPY_UNLIKELY(threads.chunk_thread_capacity == 0UL)) {
        SPOOPY_LOG_WARN("No free space for thread buffer, cannot set thread buffer, you surpassed the maximum number of threads: %zu", sizeof(threads.chunk_thread_capacity));
        return;
    }

    // What I'm doing is very dangerous
    // So, I'm checking if the thread buffer is large enough to hold the thread index
    if(SPOOPY_UNLIKELY(sizeof(thread_buffer->buffers.thread_buffer) < sizeof(spoopy_thread_index_t))) {
        SPOOPY_LOG_ERROR("Thread buffer is too small to hold thread index, minimum size is %zu bytes", sizeof(spoopy_thread_index_t));
        return;
    }

    // Very Dangerous, but we are using it to get the index of the thread buffer
    // Kids, don't try this at home!
    *(spoopy_thread_index_t*)thread_buffer->buffers.thread_buffer =
        __builtin_ctzl(threads.chunk_thread_capacity);
    spoopy_thread_index_t index = *(spoopy_thread_index_t*)thread_buffer->buffers.thread_buffer;

    // Clear the bit at the index we are setting since it's now occupied
    // This is a bit manipulation trick to clear the bit at th index
    threads.chunk_thread_capacity &= (threads.chunk_thread_capacity - 1);

    memcpy(
        &threads.global_threads[index],
        thread_buffer,
        sizeof(spoopy_global_thread_wrapper_t)
    );
}

void _spoopy_internal_thread_unset(spoopy_thread_index_t index) {
    if(threads.chunk_thread_capacity == ~0UL) {
        SPOOPY_LOG_WARN("No thread buffer set, cannot unset thread buffer");
        return;
    }

    threads.chunk_thread_capacity |= (1UL << index);

    memset(
        &threads.global_threads[index],
        0,
        sizeof(spoopy_global_thread_wrapper_t)
    );
}

spoopy_thread_index_t spoopy_get_main_id(void) {
    return threads.main_thread_id;
}
