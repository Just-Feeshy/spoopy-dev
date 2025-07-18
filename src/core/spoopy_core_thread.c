#define __SPOOPY_USE_CORE_THREAD_DESIGN
#include "spoopy_core.h"
#include <spoopy_thread.h>

spoopy_thread_manager_t threads = {0};

void _spoopy_internal_thread_set(spoopy_global_thread_wrapper_t* thread_buffer) {
    if(SPOOPY_UNLIKELY(threads.chunk_thread_capacity == 0UL)) {
        SPOOPY_LOG_WARN("No free space for thread buffer, cannot set thread buffer, you surpassed the maximum number of threads: %zu", sizeof(threads.chunk_thread_capacity));
        return;
    }

    thread_buffer->index = __builtin_ctzl(threads.chunk_thread_capacity);
    threads.chunk_thread_capacity = threads.chunk_thread_capacity
                                    & (threads.chunk_thread_capacity - 1);

    memcpy(
        &threads.global_threads[thread_buffer->index],
        thread_buffer,
        sizeof(spoopy_global_thread_wrapper_t)
    );
}

void _spoopy_internal_thread_unset(spoopy_global_thread_wrapper_t* thread_buffer) {
    if(threads.chunk_thread_capacity == ~0UL) {
        SPOOPY_LOG_WARN("No thread buffer set, cannot unset thread buffer");
        return;
    }

    unsigned long index = thread_buffer->index;
    threads.chunk_thread_capacity |= (1UL << index);

    memset(
        &threads.global_threads[index],
        0,
        sizeof(spoopy_global_thread_wrapper_t)
    );
}
